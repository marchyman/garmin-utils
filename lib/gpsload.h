/*
 *	$snafu: gpsload.h,v 1.4 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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

