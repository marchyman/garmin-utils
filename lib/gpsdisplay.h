/*
 *	$snafu: gpsdisplay.h,v 1.4 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

    /*
     * Write len bytes at buf to stderr.  The `direction' parameter is
     * intended to be '<' to show data received and '>' to show data
     * sent, but any character can be used.
     */
void gpsDisplay( char direction, const unsigned char * buf, int len );
