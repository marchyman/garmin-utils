/*
 *	$snafu: gpsprod.c,v 1.2 2001/05/02 00:34:55 marc Exp $
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

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gps2.h"
#include "gpsprod.h"

    /*
     * Issue a product request and return data from its response.
     * A product request is protocol ID 254, its response is 255.
     */
int
gpsProduct( GpsHandle gps, int * productId, int * softwareVersion,
	    char ** productDescription )
{
    char rqst = prodRqst;
    int retries = 5;
    unsigned char * data = malloc( GPS_FRAME_MAX );

    if ( ! data ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "no memory: product request" );
	}
	return -1;
    }
    if ( gpsDebug( gps ) > 2 ) {
	warnx( "send: product request" );
    }
    while ( retries-- ) {
	if ( gpsSendWait( gps, &rqst, 1 ) == 1 ) {
	    int dataLen = GPS_FRAME_MAX;
	    if ( gpsRecv( gps, 5, data, &dataLen ) == 1 ) {
		if ( data[ 0 ] == prodResp ) {
		    gpsSendAck( gps, *data );
		    *productId = data[ 1 ] + ( data[ 2 ] << 8 );
		    *softwareVersion = data[ 3 ] + ( data[ 4 ] << 8 );
		    if ( dataLen > 5 ) {
			*productDescription = strdup( &data[ 5 ] );
		    } else {
			*productDescription = 0;
		    }
		    if ( gpsDebug( gps ) > 2 ) {
			warnx( "rcvd: product data" );
		    }
		    free( data );
		    return 0;
		}
	    }
	    gpsSendNak( gps, *data );
	    if ( gpsDebug( gps ) > 2 ) {
		warnx( "retry: product request" );
	    }	
	}
    }
    if ( gpsDebug( gps ) ) {
	warnx( "fail: product request" );
    }
    free( data );
    return -1;
}
