/*
 *	$Id: gpsdisplay.h,v 1.1 1998/05/12 05:01:15 marc Exp $
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

    /*
     * Write len bytes at buf to stderr.  The `direction' parameter is
     * intended to be '<' to show data received and '>' to show data
     * sent, but any character can be used.
     */
void gpsDisplay( char direction, const unsigned char * buf, int len );
