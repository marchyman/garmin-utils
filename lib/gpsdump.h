/*
 *	$snafu: gpsdump.h,v 1.4 2001/06/13 22:21:27 marc Exp $
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
     * Garmin GPS device command and transfer protocols
     *
     * dev1 -> dev2:	command
     * dev2 -> dev1:	transfer begin
     * dev2 -> dev1:	transfer data
     * dev2 -> dev1:	transfer end
     */

    /*
     * Issue a device command and wait for an ack.  Returns
     *	-1:	command failed
     *	0:	command naked
     *	1:	command acked.
     */
int gpsCmd( GpsHandle gps, GpsCmdId cmd );

