/*
 *	$snafu: gpscap.c,v 1.3 2001/06/13 22:21:26 marc Exp $
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

#include <stdlib.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gps2.h"
#include "gpscap.h"

int
gpsProtocolCap( GpsHandle gps )
{
    int retries = 5;
    unsigned char * data = malloc( GPS_FRAME_MAX );
    int dataLen;

    if ( ! data ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "no memory: protocol capabilities" );
	}
	return -1;
    }
    if ( gpsDebug( gps ) > 2 ) {
	warnx( "recv: protocol capabilities" );
    }
    while ( retries-- ) {
	dataLen = GPS_FRAME_MAX;
	switch ( gpsRecv( gps, 3, data, &dataLen ) ) {
	  case -1:
	  case 0:
	    goto done;
	  case 1:
	    if ( data[ 0 ] == protoCap ) {
		gpsSendAck( gps, *data );
		free( data );
		if ( gpsDebug( gps ) > 2 ) {
		    warnx( "rcvd: protocol capabilities" );
		}
		return 0;
	    }
	    gpsSendNak( gps, *data );
	    if ( gpsDebug( gps ) > 2 ) {
		warnx( "retry: protocol capabilities " );
	    }
	}
    }
 done:
    free( data );
    return -1;
}
