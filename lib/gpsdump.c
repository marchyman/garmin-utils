/*
 *	$snafu: gpsdump.c,v 1.6 2003/04/10 20:50:22 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <stdlib.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gps2.h"
#include "gpsdump.h"
#include "gpsprint.h"

    /*
     * Garmin GPS device command and transfer protocols
     *
     * dev1 -> dev2:	command
     * dev2 -> dev1:	transfer begin
     * dev2 -> dev1:	transfer data
     * dev2 -> dev1:	transfer end
     */

    /*
     * Issue a device command and wait for an ack.  Returns
     *	-1:	command failed
     *	0:	command naked
     *	1:	command acked.
     */
int
gpsCmd( GpsHandle gps, GpsCmdId cmd )
{
    unsigned char cmdFrame[ 4 ];
    int retries = 5;
    unsigned char * data = malloc( GPS_FRAME_MAX );

    if ( ! data ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "no memory: gps command" );
	}
	return -1;
    }

    /* Build the command frame. */

    cmdFrame[ 0 ] = cmdType;
    cmdFrame[ 1 ] = cmd;
    cmdFrame[ 2 ] = 0;

    if ( gpsDebug( gps ) > 2 ) {
	warnx( "send: gps command %d", cmd );
    }
    while ( retries-- ) {
	if ( gpsSendWait( gps, cmdFrame, 3 ) == 1 ) {

	    /* command accepted, init data packet length and kill any
	       remaining retries */

	    int dataLen = GPS_FRAME_MAX;
	    retries = 0;

	    /* read until failure or end of transfer packet */

	    while ( gpsRecv( gps, 2, data, &dataLen ) == 1 ) {
		gpsSendAck( gps, *data );
		gpsPrint( gps, cmd, data, dataLen );
		if ( *data == xfrEnd || *data == utcData ) {
		    break;
		}
		dataLen = GPS_FRAME_MAX;
	    }

	    free( data );
	    return 1;
	}
	if ( gpsDebug( gps ) > 2 ) {
	    warnx( "retry: gps command" );
	}	
    }
    if ( gpsDebug( gps ) ) {
	warnx( "fail: product request" );
    }
    free( data );
    return -1;
}
