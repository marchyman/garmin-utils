/*
 *	$snafu: gps1.c,v 1.11 2003/04/10 18:58:26 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

/*
 * This file implements Layer 1 of the Garmin GPS protocol.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <termios.h>

#include "gpsproto.h"
#include "gpslib.h"
#include "gpsdisplay.h"

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
	int		wptType;	/* waypoint packet type */
#if SIO_TYPE == BSD
	struct termios	termios;	/* initial term settings */
#elif SIO_TYPE == Linux
	struct termio	termios;
#else
#error Unknown SIO_TYPE value
#endif
	int		bufIx;		/* index into read buffer */
	int		bufCnt;		/* number of bytes in read buffer */
	unsigned char	buf[ GPS_BUF_LEN ];
	int		wptWptType;	/* waypoint type in A1XX protocol */
	int		rteWptType;	/* waypoint type in A2XX protocol */
	int		rteRteHdr;	/* route header type in A2xx proto */
	int		rteRteLink;	/* route link type in A2xx proto */
	int		trkTrkType;	/* track point type in A3XX proto */
	int		trkTrkHdr;	/* track hdr type in A3XX proto */
};

static struct gps_state	gps_state = { 0, -1, 0, D100, { 0 }, 0, 0 };


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
	gps_state.name = strdup( port );
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
	if (ioctl(gps_state.fd, TIOCSETAF, &termios ) < 0)
		err(1, "TIOCSETAF");

#elif SIO_TYPE == Linux
	if (ioctl(gps_state.fd, TCGETA, &termios) < 0)
		err(1, "TCGETA");
	/* save current terminal settings */
	memcpy(&gps_state.termios, &termios, sizeof gps_state.termios);
	termios.c_cflag  = (CSIZE & CS8) | CREAD | (CBAUD & B9600);
	termios.c_iflag  = termios.c_oflag = termios.c_lflag = (ushort)0;
	termios.c_oflag  = (ONLRET);
	if (ioctl(gps_state.fd, TCSETAF, &termios ) < 0)
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
gps_close( gps_handle gps )
{
	if (gps == &gps_state) {
		if (gps_state.fd != -1) {
#if SIO_TYPE == BSD
			if (ioctl(gps_state.fd, TIOCSETAF,
				  &gps_state.termios) < 0)
				err(1, "TIOCSETAF");

#elif SIO_TYPE == Linux
			if (ioctl(gps_state.fd, TCSETAF,
				  &gps_state.termios ) < 0)
				err(1, "TCSETAF");

#else
#error Unknown SIO_TYPE value
#endif
			close(gps_state.fd);
			gps_state.fd = -1;
			gps_state.debug = 0;
			gps_state.bufIx = 0;
			gps_state.bufCnt = 0;
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
gps_read(gps_handle gps, unsigned char * val, int timeout)
{
    if ( gps == &gps_state ) {
	if ( gps_state.bufIx >= gps_state.bufCnt ) {
	    int stat;
	    struct timeval  time;
#if SIO_TYPE == BSD
	    struct fd_set   readfds;
#elif SIO_TYPE == Linux
	    fd_set   readfds;
#else
#error Unknown SIO_TYPE value
#endif
	    memset( &time, 0, sizeof time );
	    time.tv_sec = timeout;
	    FD_ZERO( &readfds );
	    FD_SET( gps_state.fd, &readfds );
	    do {
		stat = select( gps_state.fd + 1, &readfds, 0, 0,
			       timeout == -1 ? 0 : &time );
	    } while (( stat < 0 ) && ( errno == EINTR ));
	    switch ( stat ) {
	      case -1:
		if ( gps_state.debug ) {
		    warn( gps_state.name );
		}
		return -1;
	      case 0:
		return 0;
	      case 1:
		gps_state.bufIx = 0;
		gps_state.bufCnt =
		    read( gps_state.fd, gps_state.buf, GPS_BUF_LEN );
		if ( gps_state.bufCnt <= 0 ) {
		    if ( gps_state.debug ) {
			warn( gps_state.name );
		    }
		    return -1;
		}
		if ( gps_state.debug > 4 ) {
		    gpsDisplay( '<', gps_state.buf, gps_state.bufCnt );
		}
	    }
	}
	if ( gps_state.bufIx < gps_state.bufCnt ) {
	    *val = gps_state.buf[ gps_state.bufIx++ ];
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
gps_write(gps_handle gps, const unsigned char * buf, int cnt)
{
	int written;

	if (gps == &gps_state) {
		while (cnt > 0) {
			written = write(gps_state.fd, buf, cnt);
			if (written > 0) {
				if ( gps_state.debug > 4)
					gpsDisplay('>', buf, written);
				cnt -= written;
				buf += written;
			} else {
				if (gps_state.debug)
					warn( gps_state.name );
				return -1;
			}
		}
		return 1;
	}
	return -1;
}

void
gps_set_wpt_type(gpsHandle gps, int wptType)
{
	if (gps == &gps_state)
		gps_state.wptType = wptType;
}

int
gpsGetWptType (gps_handle gps)
{
	if (gps == &gps_state)
		return gps_state.wptType;
	return -1;
}

static void
gpsSetWptWptType(gps_handle gps, int type)
{
    if (gps == &gps_state) {
	gps_state.wptWptType = type;
    }
}

static void
gpsSetRteRteHdr(gps_handle gps, int type)
{
    if (gps == &gps_state) {
	gps_state.rteRteHdr = type;
    }
}

static void
gpsSetRteRteLink(gps_handle gps, int type)
{
    if (gps == &gps_state) {
	gps_state.rteRteLink = type;
    }
}

static void
gpsSetRteWptType(gps_handle gps, int type)
{
    if (gps == &gps_state) {
	gps_state.rteWptType = type;
    }
}

static void
gpsSetTrkTrkHdr(gps_handle gps, int type)
{
    if (gps == &gps_state) {
	gps_state.trkTrkHdr = type;
    }
}

static void
gpsSetTrkTrkType(gps_handle gps, int type)
{
    if (gps == &gps_state) {
	gps_state.trkTrkType = type;
    }
}

int
gpsGetRteRteHdr(gps_handle gps)
{
    if (gps == &gps_state) {
	return gps_state.rteRteHdr;
    }
    return (-1);
}

int
gpsGetTrkTrkType(gps_handle gps)
{
    if (gps == &gps_state) {
	return gps_state.trkTrkType;
    }
    return (-1);
}

/* XXX this probably belongs in gpscaps.c */
void
gpsSetCapability(gps_handle gps, int phys, int link, int app, int dat, int typ)
{
	if (phys == 0) {
		/* common stuff between the two link protocols */
		if (link == 1 || link == 2) {
			if (app == 100 && dat == 0) {	/* A100 <D0> */
				gpsSetWptWptType(gps, typ);
			}

			if (app == 200 && dat == 0) {	/* A200 <D0> */
				gpsSetRteRteHdr(gps, typ);
			}
			if (app == 200 && dat == 1) {	/* A200 <D1> */
				gpsSetRteWptType(gps, typ);
			}

			if (app == 201 && dat == 0) {	/* A201 <D0> */
				gpsSetRteRteHdr(gps, typ);
			}
			if (app == 201 && dat == 1) {	/* A201 <D1> */
				gpsSetRteWptType(gps, typ);
			}
			if (app == 201 && dat == 1) {	/* A201 <D2> */
				gpsSetRteRteLink(gps, typ);
			}

			if (app == 300 && dat == 0) {	/* A300 <D0> */
				gpsSetTrkTrkType(gps, typ);
			}

			if (app == 301 && dat == 0) {	/* A301 <D0> */
				gpsSetTrkTrkHdr(gps, typ);
			}
			if (app == 301 && dat == 1) {	/* A301 <D1> */
				gpsSetTrkTrkType(gps, typ);
			}

			/* XXX more ... */

		}
	}
}
