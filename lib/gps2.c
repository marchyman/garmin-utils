/*
 *	$Id: gps2.c,v 1.1 1998/05/12 05:01:15 marc Exp $
 *
 *	Copyright (c) 1998 Marco S. Hyman
 *
 *	Permission to copy all or part of this material for any purpose is
 *	granted provided that the above copyright notice and this paragraph
 *	are duplicated in all copies.  THIS SOFTWARE IS PROVIDED ``AS IS''
 *	AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 *	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *	FOR A PARTICULAR PURPOSE.
 */

    /*
     * Garming GPS protocol, data link layer
     */

#include <stdlib.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gps2.h"
#include "gpsdisplay.h"

    /*
     * Put application data into layer two frame format and return
     * a buffer with the formatted frame.  The size of the formated
     * frame is returned in the cnt parameter. Note: the buffer must be
     * freed.  Frame format is:
     *
     *	DLE
     *	record type, add to checksum
     *	data length, add to checksum, escape if DLE
     *	data, add to checksum, escape all DLE
     *	checksum, escape if DLE
     *	DLE
     *	ETX
     */
static unsigned char *
gpsFormat( const unsigned char * buf, int * cnt )
{
    int sum = 0;
    int ix = 0;
    unsigned char * work = malloc( 2 * *cnt + 10 );

    if ( ! work ) {
	warn( "gpsFormat: no memory" );
	return 0;
    }

    /* start with a dle */

    work[ ix++ ] = dle;

    /* record type, add to checksum */

    sum += *buf;
    work[ ix++ ] = *buf++;
    *cnt -= 1;

    /* data length, escape if len == dle.  Add len to checksum */

    work[ ix ] = (unsigned char) *cnt;
    sum += work[ ix ];
    if ( work[ ix++ ] == dle ) {
	work[ ix++ ] = dle;
    }

    /* copy data (if any) to buffer adding to checksum and escaping
       all dle characters */
    
    while ( (*cnt)-- ) {
	sum += *buf;
	if ( *buf == dle ) {
	    work[ ix++ ] = dle;
	}
	work[ ix++ ] = *buf++;
    }

    /* add the neg of the checksum. */

    work[ ix ] = (unsigned char) (-sum);
    if ( work[ ix++ ] == dle ) {
	work[ ix++ ] = dle;
    }

    /* add the final dle/etx */

    work[ ix++ ] = dle;
    work[ ix++ ] = etx;
    *cnt = ix;
    return work;
}
    
    /*
     * Send a frame containing the given data.  The first byte of the data
     * is assumed to be the garmin record type.
     *
     */
int
gpsSend( GpsHandle gps, const unsigned char * buf, int cnt )
{
    int ok = -1;
    int len = cnt;
    unsigned char * data = gpsFormat( buf, &len );

    if ( data ) {
	if ( gpsDebug( gps ) > 3 ) {
	    gpsDisplay( '}', buf, cnt );
	}
	ok = gpsWrite( gps, data, len );
	free( data );
    }
    return ok;
}

int
gpsSendAck( GpsHandle gps, unsigned char type )
{
    unsigned char buf[ 4 ];

    buf[ 0 ] = ack;
    buf[ 1 ] = type;
    buf[ 2 ] = 0;
    return gpsSend( gps, buf, 3 );
}

int
gpsSendNak( GpsHandle gps, unsigned char type )
{
    unsigned char buf[ 4 ];

    buf[ 0 ] = nak;
    buf[ 1 ] = type;
    buf[ 2 ] = 0;
    return gpsSend( gps, buf, 3 );
}

int
gpsRecv( GpsHandle gps, int to, unsigned char * buf, int * cnt )
{
    int dleSeen;
    int etxSeen;
    int sum;
    int len;
    int rlen = -1;
    unsigned char * ptr;
    int stat;

    /* sync to the first DLE to come down the pike. */

    while ( 1 ) {
	do {
	    stat = gpsRead( gps, buf, to );
	} while (( stat == 1 ) && ( *buf != dle ));

	/* We have a timeout or a frame (or possibly the middle or end of a
	   packet). If a timeout return a -1, otherwise prepare to receive
	   the rest of the frame */

	switch ( stat ) {
	  case -1:
	    if ( gpsDebug( gps ) > 1 ) {
		warnx( "sync error: gps recv" );
	    }
	    return -1;
	  case 0:
	    if ( gpsDebug( gps ) > 1 ) {
		warnx( "timeout: gps recv" );
	    }
	    return 0;
	  case 1:
	    break;
	}

	/* start receiving characters into buf.  An end of buffer or a DLE ETX
	   sequence will terminate the reception.  Each read is given a 10
	   second timeout -- if we time out assume the gps died and return
	   an error. */

	ptr = buf;
	dleSeen = 0;
	etxSeen = 0;
	sum = 0;
	len = 0;
	do {
	    stat = gpsRead( gps, ptr, 10 );
	    if ( stat != 1 ) {
		if ( gpsDebug( gps ) > 1 ) {
		    warnx( "frame error: gps recv" );
		}
		return -1;
	    }
	    if ( dleSeen ) {
		if ( *ptr == etx ) {
		    etxSeen = 1;
		    break;
		}
		dleSeen = 0;
	    } else {
		if ( *ptr == dle ) {
		    dleSeen = 1;
		    continue;
		}
	    }
	    if (( rlen == -1 ) && ( len == 1 )) {
		/* this is the length byte, add it to the checksum and
		   save it, but do not keep it in the buffer */
		sum += *ptr;
		rlen = *ptr;
	    } else {
		sum += *ptr++;
		len += 1;
	    }
	} while ( len < *cnt );
	if ( etxSeen ) {
	    /* subtract one from the length as we don't count the
	       checksum. */
	    len -= 1;

	    /* warn if the length is not the expected value.  Add in the
	       packet type to the expected length. */

	    rlen += 1;
	    if ( gpsDebug( gps ) && ( rlen != len )) {
		warnx( "bad frame length, %d expected, %d received",
		       rlen, len );
	    }
	    if (( sum & 0xff ) == 0 ) {
		/* good checksum, update lenght rcvd and return */
		*cnt = len;
		if ( gpsDebug( gps ) > 3 ) {
		    gpsDisplay( '{', buf, len );
		}
		return 1;
	    } else {
		/* bad checksum -- try again */
		if ( gpsDebug( gps ) > 3 ) {
		    gpsDisplay( '!', buf, len );
		}
	    }
	} else {
	    /* frame too large, return error */
	    if ( gpsDebug( gps ) ) {
		warnx( "frame too large for %d byte buffer", *cnt );
	    }
	    return -1;
	}
    }
}

int
gpsSendWait( GpsHandle gps, const unsigned char * buf, int cnt )
{
    int retry = 5;
    int ok = -1;
    int len = cnt;
    unsigned char * data = gpsFormat( buf, &len );
    unsigned char * response = malloc( GPS_FRAME_MAX );
    int respLen;

    if ( data && response ) {
	if ( gpsDebug( gps ) > 3 ) {
	    gpsDisplay( '}', buf, cnt );
	}
	while ( retry-- ) {
	    if ( gpsWrite( gps, data, len ) == 1 ) {
		respLen = GPS_FRAME_MAX;
		if ( gpsRecv( gps, 2, response, &respLen ) == 1 ) {
		    if (( respLen > 2 ) &&
			( response[ 0 ] == ack ) &&
			( response[ 1 ] == *buf )) { 
			ok = 1;
			break;
		    }
		}
		if ( gpsDebug( gps ) > 1 ) {
		    warnx( "retry: send and wait" );
		}
	    }
	}
    }
    if ( data ) {
	free( data );
    }
    if ( response ) {
	free( response );
    }
    return ok;
}
