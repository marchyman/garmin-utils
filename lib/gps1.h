/*
 *	$Id: gps1.h,v 1.1 1998/05/12 05:01:15 marc Exp $
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


