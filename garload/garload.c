/*
 *	$Id: garload.c,v 1.1 1998/05/12 05:01:15 marc Exp $
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

static void
usage( const char* prog, const char* err, ... )
{
    if ( err ) {
	va_list ap;
	va_start( ap, err );
	vfprintf( stderr, err, ap);
	va_end( ap );
    }
    fprintf( stderr, "usage: %s [-d debug-level] [-wrt] [-p port]\n", prog );
    exit( 1 );
}

int
main( int argc, char * argv[] )
{
    int debug = 0;
    const char* port = "/dev/cua00";
    int opt;
    char* rem;

    while (( opt = getopt( argc, argv, "d:p:" )) != -1 ) {
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
    ;;;
    return 0;
}
