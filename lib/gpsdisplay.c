/*
 *	$snafu: gpsdisplay.c,v 1.5 2001/06/19 04:36:47 marc Exp $
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

