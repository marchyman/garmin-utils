/*
 *	$snafu: gpsprod.h,v 1.4 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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
