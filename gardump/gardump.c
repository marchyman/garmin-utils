/*
 *	$snafu: gardump.c,v 1.5 2001/05/02 00:34:51 marc Exp $
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
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gpsversion.h"
#include "gpsdump.h"

static void
usage( const char* prog, const char* err, ... )
{
    if ( err ) {
	va_list ap;
	va_start( ap, err );
	vfprintf( stderr, err, ap);
	va_end( ap );
    }
    fprintf( stderr, "usage: %s [-d debug-level] [-vwrt] [-p port]\n", prog );
    exit( 1 );
}

int
main( int argc, char * argv[] )
{
    int waypoints = 0;
    int routes = 0;
    int tracks = 0;
    int debug = 0;
    const char* port = DEFAULT_PORT;

    int opt;
    char* rem;
    GpsHandle gps;

    while (( opt = getopt( argc, argv, "d:vwrtp:" )) != -1 ) {
	switch ( opt ) {
	  case 'd':
	    debug = strtol( optarg, &rem, 0 );
	    if ( *rem ) {
		debug = 0;
	    }
	    if ( ! debug ) {
		usage( argv[ 0 ], "`%s' is a bad debug value\n", optarg );
		/* does not return */
	    }
	    if ( debug > 1 ) {
		warnx( "debug level set to %d", debug );
	    }
	    break;
	  case 'v':
	    errx( 1, "software version %s", VERSION );
	    /* does not return */
	  case 'w':
	    waypoints = 1;
	    if ( debug > 1 ) {
		warnx( "waypoint dump requested" );
	    }
	    break;
	  case 'r':
	    routes = 1;
	    if ( debug > 1 ) {
		warnx( "route dump requested" );
	    }
	    break;
	  case 't':
	    if ( debug > 1 ) {
		warnx( "track dump requested" );
	    }
	    tracks = 1;
	    break;
	  case 'p':
	    port = strdup( optarg );
	    if ( debug > 1 ) {
		warnx( "using serial port `%s'", port );
	    }
	    break;
	  case '?':
	  default:
	    usage( argv[ 0 ], 0 );
	    /* does not return */
	}
    }

    if ( argc != optind ) {
	errx( 1, "unknown command line argument: %s ...", argv[ optind ] );
	/* does not return */
    }

    if (( ! waypoints ) && ( ! routes ) && ( ! tracks )) {
	waypoints = routes = tracks = 1;
	if ( debug > 1 ) {
	    warnx( "waypoint, route, and track dump requested" );
	}
    }

    gps = gpsOpen( port, debug );
    if ( gpsVersion( gps ) != 1 ) {
	errx( 1, "can't communicate with GPS unit" );
	/* does not return */
    }
    if ( waypoints ) {
	gpsCmd( gps, CMD_WPT );
    }
    if ( routes ) {
	gpsCmd( gps, CMD_RTE );
    }
    if ( tracks ) {
	gpsCmd( gps, CMD_TRK );
    }
    gpsClose( gps );

    return 0;
}

