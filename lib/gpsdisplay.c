/*
 *	$snafu: gpsdisplay.c,v 1.6 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <ctype.h>
#include <stdio.h>
#include <strings.h>

#include "gpsdisplay.h"

    /*
     * Function and variables to dump data to stderr
     */
#define DUMP_BUFLEN		80
#define DUMP_DEC_OFF		4
#define DUMP_ASCII_OFF		(DUMP_DEC_OFF + 44)

void
gpsDisplay( char direction, const unsigned char * buf, int len )
{
    unsigned char	data[ DUMP_BUFLEN ];
    int			start;

    start = 0;
    memset( data, ' ', DUMP_BUFLEN );
    while ( len ) {
	const unsigned char*	d;
	unsigned char*		h;
	unsigned char*		a;
	int			cnt;
	int			sent;

	d = buf;
	h = &data[ DUMP_DEC_OFF ];
	a = &data[ DUMP_ASCII_OFF ];
	sent = 0;

	if ( len > 10 ) {
	    cnt = 10;
	    len -= 10;
	} else {
	    cnt = len;
	    len = 0;
	    memset( data, ' ', DUMP_BUFLEN );
	}
	buf += 10;

	data[ 1 ] = direction;
	while ( cnt-- ) {
	    if ( isprint( *d ) ) {
		*a++ = *d;
	    } else {
		*a++ = '.';
	    }   
            *h++ = 48 + *d / 100;
            *h++ = 48 + *d % 100 / 10;
            *h++ = 48 + *d % 10;
	    *h++ = ' ';
	    if ( ++sent == 5 ) {
		*a++ = ' ';
		*h++ = ' ';
	    };
	    d++;
	}
	*a = 0;
	fprintf( stderr, "%s\n", data );
    }
}

