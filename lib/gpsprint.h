/*
 *	$snafu: gpsprint.h,v 1.3 2001/06/13 22:21:27 marc Exp $
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

    /*
     * Functions to `print' gps data.  The given packets are formatted
     * and written to stdout.  Formatting varies according to the
     * packet type.
     */
int gpsPrint( GpsHandle gps, GpsCmdId cmd, const unsigned char * packet,
	      int len );
