/*
 *	$snafu: gpsprint.h,v 1.4 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

    /*
     * Functions to `print' gps data.  The given packets are formatted
     * and written to stdout.  Formatting varies according to the
     * packet type.
     */
int gpsPrint( GpsHandle gps, GpsCmdId cmd, const unsigned char * packet,
	      int len );
