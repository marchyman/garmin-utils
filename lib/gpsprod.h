/*
 *	$snafu: gpsprod.h,v 1.3 2001/06/13 22:21:27 marc Exp $
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
     * Garmin GPS product data application protocol
     *
     * host -> gps:	product request
     * gps -> host:	product data
     */

    /*
     * Retrieve the product information from the unit specified by
     * `gps' and return product information in productId, softwareVersion,
     * and productDescription.  *productDescription is allocated via
     * malloc and should be free'd when no longer needed.
     *
     * procedure returns -1 on error, otherwise 0.
     */
int gpsProduct( GpsHandle gps, int * productId, int * softwareVersion,
		char ** productDescription );
