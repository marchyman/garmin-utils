/*
 *	$Id: gpsload.h,v 1.1 1998/05/14 01:38:46 marc Exp $
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
     * Garmin GPS load protocol
     *
     * dev1 -> dev2:	transfer begin
     * dev1 -> dev2:	transfer data
     * dev1 -> dev2:	transfer end
     */

    /*
     * Load the lists specified.  Return 1 if upload successful,
     * -1 otherwise.
     */
int gpsLoad( GpsHandle gps, GpsLists * lists );

