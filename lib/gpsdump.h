/*
 *	$snafu: gpsdump.h,v 1.5 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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

