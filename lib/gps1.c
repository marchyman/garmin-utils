/*
 *	$snafu: gps1.c,v 1.7 2001/06/13 22:21:26 marc Exp $
 *
 *	Copyright (c) 1998 Marco S. Hyman
 *
 *	Permission to copy all or part of this material with or without
 *	modification for any purpose is granted provided that the above
 *	copyright notice and this paragraph are duplicated in all copies.
 *
 *	THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
#include "gps1.h"
#include "gpsdisplay.h"

    /*
     * This program is designed to talk to a single gps unit.  Thus,
     * all state for a connection to the unit is kept in a single
     * static structure.  The address of this structure is the "handle".
     */
typedef struct {
    int			debug;		/* debugging level (set at open) */
    int			fd;		/* fd of the open file */
    char*		name;		/* name of the device */
#if SIO_TYPE == 0 /* BSD */
    struct termios	termios;	/* initial term settings */
#elif SIO_TYPE == 1 /* Linux */
    struct termio	termios;
#else
#error Unknown SIO_TYPE value
#endif
    int			bufIx;		/* index into read buffer */
    int			bufCnt;		/* number of bytes in read buffer */
    unsigned char	buf[ GPS_BUF_LEN ];
} GpsState;

static GpsState	gpsState = { 0, -1, 0, { 0 }, 0, 0 };


    /*
     * If we can open the requested port set params for communications
     * and return our `handle'.  The port is opened using O_NONBLOCK
     * as the garmin cable doesn't seem to supply modem control signals.
     */
GpsHandle
gpsOpen( const char * port, int debug )
{
#if SIO_TYPE == 0 /* BSD */
    struct termios  termios;
#elif SIO_TYPE == 1 /* Linux */
    struct termio   termios;
#else
#error Unknown SIO_TYPE value
#endif
    gpsState.debug = debug;
    gpsState.name = strdup( port );
    if ( ! gpsState.name ) {
	err( 1, "serial port name too large" );
	/* does not return */
    }
    gpsState.fd = open( gpsState.name, O_RDWR | O_NONBLOCK );
    if ( gpsState.fd == -1 ) {
	errx( 1, "can't open gps device `%s': %s", gpsState.name,
	      strerror( errno ) );
	/* does not return */
    }

#if SIO_TYPE == 0 /* BSD */
    if ( ioctl( gpsState.fd, TIOCGETA, &termios) < 0 ) {
	err( 1, "TIOCGETA" );
	/* does not return */
    }
    /* save current terminal settings */
    memcpy( &gpsState.termios, &termios, sizeof gpsState.termios );
    termios.c_ispeed = termios.c_ospeed = 9600;
    termios.c_iflag = 0;
    termios.c_oflag = 0;	/* (ONLRET) */
    termios.c_cflag = CS8 | CREAD | CLOCAL;
    termios.c_lflag = 0;
    memset( termios.c_cc, -1, NCCS );
    termios.c_cc[VMIN] = 1;
    termios.c_cc[VTIME] = 0;
    if ( ioctl( gpsState.fd, TIOCSETAF, &termios ) < 0) {
	err( 1, "TIOCSETAF" );
	/* does not return */
    }
#elif SIO_TYPE == 1 /* Linux */
    if ( ioctl( gpsState.fd, TCGETA, &termios) < 0 ) err( 1, "TCGETA" );
    /* save current terminal settings */
    memcpy( &gpsState.termios, &termios, sizeof gpsState.termios );
    termios.c_cflag  = (CSIZE & CS8) | CREAD | (CBAUD & B9600);
    termios.c_iflag  = termios.c_oflag = termios.c_lflag = (ushort)0;
    termios.c_oflag  = (ONLRET);
    if ( ioctl( gpsState.fd, TCSETAF, &termios ) < 0) err( 1, "TCSETAF" );
#else
#error Unknown SIO_TYPE value
#endif
    return &gpsState;
}

void
gpsClose( GpsHandle gps )
{
    if ( gps == &gpsState ) {
	if ( gpsState.fd != -1 ) {
#if SIO_TYPE == 0 /* BSD */
	    if ( ioctl( gpsState.fd, TIOCSETAF, &gpsState.termios ) < 0) {
		err( 1, "TIOCSETAF" );
		/* does not return */
	    }
#elif SIO_TYPE == 1 /* Linux */
	    if ( ioctl( gpsState.fd, TCSETAF, &gpsState.termios ) < 0) {
		err( 1, "TCSETAF" );
		/* does not return */
	    }
#else
#error Unknown SIO_TYPE value
#endif
	    close( gpsState.fd );
	    gpsState.fd = -1;
	    gpsState.debug = 0;
	    gpsState.bufIx = 0;
	    gpsState.bufCnt = 0;
	    free( gpsState.name );
	    return;
	}
	if ( gpsState.debug ) {
	    warnx( "gpsClose called when no file opened" );
	}
	return;
    }
    if ( gpsState.debug ) {
	warnx( "gpsClose called with invalid handle" );
    }
}

int
gpsDebug( GpsHandle gps )
{
    if ( gps == &gpsState ) {
	return gpsState.debug;
    }
    return 0;
}

    /*
     * If we have a character queued, return it.  If not read up to
     * GPS_BUF_LEN characters from the device with the given timeout
     * and return the first character read.  If no chararcters read
     * return timeout.
     */
int
gpsRead( GpsHandle gps, unsigned char * val, int timeout )
{
    if ( gps == &gpsState ) {
	if ( gpsState.bufIx >= gpsState.bufCnt ) {
	    int stat;
	    struct timeval  time;
#if SIO_TYPE == 0 /* BSD */
	    struct fd_set   readfds;
#elif SIO_TYPE == 1 /* Linux */
	    fd_set   readfds;
#else
#error Unknown SIO_TYPE value
#endif
	    memset( &time, 0, sizeof time );
	    time.tv_sec = timeout;
	    FD_ZERO( &readfds );
	    FD_SET( gpsState.fd, &readfds );
	    do {
		stat = select( gpsState.fd + 1, &readfds, 0, 0,
			       timeout == -1 ? 0 : &time );
	    } while (( stat < 0 ) && ( errno == EINTR ));
	    switch ( stat ) {
	      case -1:
		if ( gpsState.debug ) {
		    warn( gpsState.name );
		}
		return -1;
	      case 0:
		return 0;
	      case 1:
		gpsState.bufIx = 0;
		gpsState.bufCnt =
		    read( gpsState.fd, gpsState.buf, GPS_BUF_LEN );
		if ( gpsState.bufCnt <= 0 ) {
		    if ( gpsState.debug ) {
			warn( gpsState.name );
		    }
		    return -1;
		}
		if ( gpsState.debug > 4 ) {
		    gpsDisplay( '<', gpsState.buf, gpsState.bufCnt );
		}
	    }
	}
	if ( gpsState.bufIx < gpsState.bufCnt ) {
	    *val = gpsState.buf[ gpsState.bufIx++ ];
	    return 1;
	}
    }
    return -1;
}

int
gpsWrite( GpsHandle gps, const unsigned char * buf, int cnt )
{
    int written;

    if ( gps == &gpsState ) {
	while ( cnt > 0 ) {
	    written = write( gpsState.fd, buf, cnt );
	    if ( written > 0 ) {
		if ( gpsState.debug > 4 ) {
		    gpsDisplay( '>', buf, written );
		}
		cnt -= written;
		buf += written;
	    } else {
		if ( gpsState.debug ) {
		    warn( gpsState.name );
		}
		return -1;
	    }
	}
	return 1;
    }
    return -1;
}
