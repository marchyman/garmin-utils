/*
 *	$snafu: gpscap.h,v 1.2 2001/05/02 00:34:53 marc Exp $
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
     * Garmin GPS protocol capability application protocol
     *
     * gps -> host:	protocol array
     */

    /*
     * See if the gps unit will send the supported protocol array.
     * Garmin says that some products will send this immediatly
     * following the product data protocol.  We'll wait about
     * 5 seconds for the data.  If it is not found assume it is
     * not supported.
     *
     * procedure returns -1 on error, otherwise 0.
     */
int gpsProtocolCap( GpsHandle gps );
