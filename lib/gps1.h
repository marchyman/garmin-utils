/*
 *	$snafu: gps1.h,v 1.5 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

    /*
     * Garmin GPS layer 1 functions.
     */

typedef void * GpsHandle;

    /*
     * Open the named port and return a handle used for subsequent I/O calls
     * on this port.  If the open fails the program is aborted with an
     * error message and the function does not return.  debug is the debug
     * level from the -d command line option. 
     */
GpsHandle gpsOpen( const char * port, int debug );

    /*
     * Close the port indicated by the given handle.
     */
void gpsClose( GpsHandle gps );

    /*
     * Return the debug level associated with the given GpsHandle.
     */
int gpsDebug( GpsHandle gps );

    /*
     * Put the next character available from the requested handle into
     * `val' and return the read status.  If no character available wait
     * for up to timeout seconds.  Timeout may be 0 to poll or -1 to block
     * until a character is available.
     * Returns:
     *	-1:	read error occurred
     *	0:	timeout
     *	1:	character returned in *val.
     */
int gpsRead( GpsHandle gps, unsigned char * val, int timeout );

    /*
     * Write the requested buffer to the device indicated by the passed
     * handle and return the write status.
     */
int gpsWrite( GpsHandle gps, const unsigned char * buf, int cnt );

void gpsSetWptType (GpsHandle gps, int wptType);
int  gpsGetWptType (GpsHandle gps);
