/*
 *	$snafu: gpsversion.h,v 1.3 2001/06/13 22:21:27 marc Exp $
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
     * Garmin GPS version request and display
     */

    /*
     * Request the unit version and output to stdout.  Wait for a
     * protocol capabilities packet and consume it if seen.
     *
     *	-1:	command failed
     *	1:	command succeeded.
     */
int gpsVersion( GpsHandle gps );

