/*
 * $snafu: gps1.c,v 1.15 2003/06/12 16:45:00 marc Exp $
 *
 * Public Domain, 2001, Marco S Hyman <marc@snafu.org>
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "gpslib.h"

/*
 * Define the various serial I/O types
 */
#define BSD	0
#define Linux	1


/*
 * This program is designed to talk to a single gps unit.  Thus,
 * all state for a connection to the unit is kept in a single
 * static structure.  The address of this structure is the "handle".
 */
struct gps_state {
	int		debug;		/* debugging level (set at open) */
	int		fd;		/* fd of the open file */
	char*		name;		/* name of the device */
#if SIO_TYPE == BSD
	struct termios	termios;	/* initial term settings */
#elif SIO_TYPE == Linux
	struct termio	termios;
#else
#error Unknown SIO_TYPE value
#endif
	int		bufix;		/* index into read buffer */
	int		bufcnt;		/* number of bytes in read buffer */
	u_char		buf[GPS_BUF_LEN];
	int		wpt_type;	/* waypoint packet type */
	int		rte_hdr_type;	/* route header type */
	int		rte_wpt_type;	/* route waypoint type */
	int		rte_lnk_type;	/* route link type */
	int		trk_hdr_type;	/* track header type */
	int		trk_type;	/* track entry type */
};

static struct gps_state	gps_state = { 0, -1 };


/*
 * Open the named port and return a handle used for subsequent I/O calls
 * on this port.  If the open fails the program is aborted with an
 * error message and the function does not return.  debug is the debug
 * level from the -d command line option. 

 * If we can open the requested port set params for communications
 * and return our `handle'.  The port is opened using O_NONBLOCK
 * as the garmin cable doesn't seem to supply modem control signals.
 */
gps_handle
gps_open(const char * port, int debug)
{
#if SIO_TYPE == BSD
	struct termios  termios;
#elif SIO_TYPE == Linux
	struct termio   termios;
#else
#error Unknown SIO_TYPE value
#endif
	gps_state.debug = debug;
	gps_state.name = strdup(port);
	if (!gps_state.name)
		err(1, "serial port name too large");
	gps_state.fd = open(gps_state.name, O_RDWR | O_NONBLOCK);
	if (gps_state.fd == -1)
		errx(1, "can't open gps device `%s': %s", gps_state.name,
		      strerror(errno));

#if SIO_TYPE == BSD
	if (ioctl(gps_state.fd, TIOCGETA, &termios) < 0)
		err(1, "TIOCGETA");
	/* save current terminal settings */
	memcpy(&gps_state.termios, &termios, sizeof gps_state.termios);
	termios.c_ispeed = termios.c_ospeed = 9600;
	termios.c_iflag = 0;
	termios.c_oflag = 0;	/* (ONLRET) */
	termios.c_cflag = CS8 | CREAD | CLOCAL;
	termios.c_lflag = 0;
	memset(termios.c_cc, -1, NCCS);
	termios.c_cc[VMIN] = 1;
	termios.c_cc[VTIME] = 0;
	if (ioctl(gps_state.fd, TIOCSETAF, &termios) < 0)
		err(1, "TIOCSETAF");

#elif SIO_TYPE == Linux
	if (ioctl(gps_state.fd, TCGETA, &termios) < 0)
		err(1, "TCGETA");
	/* save current terminal settings */
	memcpy(&gps_state.termios, &termios, sizeof gps_state.termios);
	termios.c_cflag  = (CSIZE & CS8) | CREAD | (CBAUD & B9600);
	termios.c_iflag  = termios.c_oflag = termios.c_lflag = (ushort)0;
	termios.c_oflag  = (ONLRET);
	if (ioctl(gps_state.fd, TCSETAF, &termios) < 0)
		err(1, "TCSETAF");

#else
#error Unknown SIO_TYPE value
#endif
	return &gps_state;
}

/*
 * Close the port indicated by the given handle.
 */
void
gps_close(gps_handle gps)
{
	if (gps == &gps_state) {
		if (gps_state.fd != -1) {
#if SIO_TYPE == BSD
			if (ioctl(gps_state.fd, TIOCSETAF,
				  &gps_state.termios) < 0)
				err(1, "TIOCSETAF");

#elif SIO_TYPE == Linux
			if (ioctl(gps_state.fd, TCSETAF,
				  &gps_state.termios) < 0)
				err(1, "TCSETAF");

#else
#error Unknown SIO_TYPE value
#endif
			close(gps_state.fd);
			gps_state.fd = -1;
			gps_state.debug = 0;
			gps_state.bufix = 0;
			gps_state.bufcnt = 0;
			free(gps_state.name);
			return;
		}
		if (gps_state.debug)
			warnx("gpsClose called when no file opened");
		return;
	}
	if (gps_state.debug)
		warnx("gpsClose called with invalid handle");
}

/*
 * Return the debug level associated with the given gps_handle.
 */
int
gps_debug(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.debug;
	return 0;
}

/*
 * Put the next character available from the requested handle into
 * `val' and return the read status.  If no character available wait
 * for up to timeout seconds.  Timeout may be 0 to poll or -1 to block
 * until a character is available.
 * Returns:
 *	-1:	read error occurred
 *	0:	timeout
 *	1:	character returned in *val.
 *
 * If we have a character queued, return it.  If not read up to
 * GPS_BUF_LEN characters from the device with the given timeout
 * and return the first character read.  If no chararcters read
 * return timeout.
 */
int
gps_read(gps_handle gps, u_char * val, int timeout)
{
	if (gps == &gps_state) {
		if (gps_state.bufix >= gps_state.bufcnt) {
			int stat;
			struct timeval  time;
#if SIO_TYPE == BSD
			struct fd_set   readfds;
#elif SIO_TYPE == Linux
			fd_set   readfds;
#else
#error Unknown SIO_TYPE value
#endif
			memset(&time, 0, sizeof time);
			time.tv_sec = timeout;
			FD_ZERO(&readfds);
			FD_SET(gps_state.fd, &readfds);
			do {
				stat = select(gps_state.fd + 1, &readfds, 0, 0,
					      timeout == -1 ? 0 : &time);
			} while ((stat < 0) && (errno == EINTR));
			switch (stat) {
			case -1:
				if (gps_state.debug)
					warn(gps_state.name);
				return -1;
			case 0:
				return 0;
			case 1:
				gps_state.bufix = 0;
				gps_state.bufcnt =
					read(gps_state.fd, gps_state.buf,
					     GPS_BUF_LEN); 
				if (gps_state.bufcnt <= 0) {
					if (gps_state.debug)
						warn(gps_state.name);
					return -1;
				}
				if (gps_state.debug > 4) {
					gps_display('<', gps_state.buf,
						    gps_state.bufcnt);
				}
			}
		}
		if (gps_state.bufix < gps_state.bufcnt) {
			*val = gps_state.buf[gps_state.bufix++];
			return 1;
		}
	}
	return -1;
}

/*
 * Write the requested buffer to the device indicated by the passed
 * handle and return the write status.
 */
int
gps_write(gps_handle gps, const u_char * buf, int cnt)
{
	int written;

	if (gps == &gps_state) {
		while (cnt > 0) {
			written = write(gps_state.fd, buf, cnt);
			if (written > 0) {
				if (gps_state.debug > 4)
					gps_display('>', buf, written);
				cnt -= written;
				buf += written;
			} else {
				if (gps_state.debug)
					warn(gps_state.name);
				return -1;
			}
		}
		return 1;
	}
	return -1;
}

void
gps_set_wpt_type(gps_handle gps, int wpt_type)
{
	if (gps == &gps_state)
		gps_state.wpt_type = wpt_type;
}

int
gps_get_wpt_type(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.wpt_type;
	return -1;
}

void
gps_set_rte_hdr_type(gps_handle gps, int type)
{
	if (gps == &gps_state)
		gps_state.rte_hdr_type = type;
}

int
gps_get_rte_hdr_type(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.rte_hdr_type;
	return -1;
}

void
gps_set_rte_wpt_type(gps_handle gps, int type)
{
	if (gps == &gps_state)
		gps_state.rte_wpt_type = type;
}

int
gps_get_rte_wpt_type(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.rte_wpt_type;
	return -1;
}

void
gps_set_rte_lnk_type(gps_handle gps, int type)
{
	if (gps == &gps_state)
		gps_state.rte_lnk_type = type;
}

int
gps_get_rte_lnk_type(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.rte_lnk_type;
	return -1;
}

void
gps_set_trk_hdr_type(gps_handle gps, int type)
{
	if (gps == &gps_state)
		gps_state.trk_hdr_type = type;
}

int
gps_get_trk_hdr_type(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.trk_hdr_type;
	return -1;
}

void
gps_set_trk_type(gps_handle gps, int type)
{
	if (gps == &gps_state)
		gps_state.trk_type = type;
}

int
gps_get_trk_type(gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.trk_type;
	return -1;
}


