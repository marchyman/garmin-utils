/*
 *	$snafu: gpsversion.h,v 1.4 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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

