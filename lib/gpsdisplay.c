/*
 *	$Id: gpsdisplay.c,v 1.1 1998/05/12 05:01:15 marc Exp $
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

#include <ctype.h>
#include <stdio.h>

#include "gpsdisplay.h"

    /*
     * Function and variables to dump data to stderr in hex format
     */
#define DUMP_BUFLEN		80
#define DUMP_HEX_OFF		4
#define DUMP_ASCII_OFF		(DUMP_HEX_OFF + ( 16 * 3 ) + 4 )

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
	h = &data[ DUMP_HEX_OFF ];
	a = &data[ DUMP_ASCII_OFF ];
	sent = 0;

	if ( len > 16 ) {
	    cnt = 16;
	    len -= 16;
	} else {
	    cnt = len;
	    len = 0;
	    memset( data, ' ', DUMP_BUFLEN );
	}
	buf += 16;

	data[ 1 ] = direction;
	while ( cnt-- ) {
	    if ( isprint( *d ) ) {
		*a++ = *d;
	    } else {
		*a++ = '.';
	    }   
	    *h++ = "0123456789abcdef"[ ( *d >> 4 ) & 0xf ];
	    *h++ = "0123456789abcdef"[ *d++ & 0xf ];
	    *h++ = ' ';
	    if ( ++sent == 8 ) {
		*a++ = ' ';
		*h++ = ' ';
	    }
	}
	*a = 0;
	fprintf( stderr, "%s\n", data );
    }
}

