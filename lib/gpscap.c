/*
 *	$snafu: gpscap.c,v 1.6 2003/04/10 18:58:27 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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
    int pntr;
    int tag, val;

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
		int phys = -1, link = -1, app = -1, dat = -1, found = 0;

                for (pntr = 1; pntr + 5 < dataLen; pntr += 3) {
                    tag = data [pntr];
                    val = data [pntr + 1] + (data [pntr + 2] << 8);
		    switch (tag) {
		    case 'P':
			phys = val;
			link = app = dat = -1;
			break;
		    case 'L':
			link = val;
			app = dat = -1;
			break;
		    case 'A':
			app = val;
			dat = -1;
			break;
		    case 'D':
			gpsSetCapability(gps, phys, link, app, ++dat, val);
			break;
		    default:
			warnx ("unknown capability tag %d\n", tag);
			break;
		    }
		    if (app == 100 && tag == 'D' && dat == 0 && !found) {
                        gpsSetWptType (gps, val);
			found = 1;
                        if (gpsDebug (gps) > 1) {
			    warnx ("waypoint packet type is %d", val);
                        }
		    }
                }
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
