/*
 *	$snafu: gpsversion.c,v 1.5 2003/04/11 01:21:49 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gpslib.h"

    /*
     * Request the unit version and output to stdout.  Wait for a
     * protocol capabilities packet and consume it if seen.
     *
     *	-1:	command failed
     *	1:	command succeeded.
     */
int
gpsVersion( GpsHandle gps )
{
    int productId;
    int softwareVersion;
    char * productDescription;

    if ( gpsProduct( gps, &productId, &softwareVersion,
		     &productDescription ) ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "can't get gps version info" );
	}
	return -1;
    }
    printf( "[product %d, version %d: %s]\n", productId, softwareVersion,
	    productDescription ? productDescription : "unknown" );
    if ( productDescription ) {
	free( productDescription );
    }

    /* Grab the protocol capabilities packet if it is there so it doesn't
       screw up anything else.  We don't use it.  Some units send it
       every time the product description is requested. */

    gpsProtocolCap( gps );
    return 1;
}
