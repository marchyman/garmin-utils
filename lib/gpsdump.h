/*
 *	$Id: gpsdump.h,v 1.1 1998/05/12 05:01:15 marc Exp $
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
     * Garmin GPS device command and transfer protocols
     *
     * dev1 -> dev2:	command
     * dev2 -> dev1:	transfer begin
     * dev2 -> dev1:	transfer data
     * dev2 -> dev1:	transfer end
     */

typedef enum {
    CMD_ABORT_XFR = 0,
    CMD_GET_RTE = 4,
    CMD_GET_TRK = 6,
    CMD_GET_WPT = 7
} GpsCmdId;

    /*
     * Issue a device command and wait for an ack.  Returns
     *	-1:	command failed
     *	0:	command naked
     *	1:	command acked.
     */
int gpsCmd( GpsHandle gps, GpsCmdId cmd );

