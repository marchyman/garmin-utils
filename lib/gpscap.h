/*
 *	$snafu: gpscap.h,v 1.4 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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
