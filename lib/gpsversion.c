/*
 *	$snafu: gpsversion.c,v 1.2 2001/05/02 00:34:55 marc Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gps1.h"
#include "gpsprod.h"
#include "gpscap.h"
#include "gpsversion.h"

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
