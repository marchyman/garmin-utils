/*
 *	$Id: gps2.h,v 1.1 1998/05/12 05:01:15 marc Exp $
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
     * Garmin GPS layer 2 functions.
     */

    /*
     * Size of an application data buffer.  This is large enough to
     * hold the frame type plus the largest allowable frame.
     */
#define GPS_FRAME_MAX	256

    /*
     * Send a buffer containing layer 3 data using garmin layer 2 framing
     * to the device indicated by the GpsHandle.
     *
     *	returns 1 if data sent and acknowledged, -1 if any errors occured.
     */
int gpsSend( GpsHandle gps, const unsigned char * buf, int cnt );

    /*
     * Send an ack for the given packet type.  Return 1 if the packet
     * sent OK, othewise -1.
     */
int gpsSendAck( GpsHandle gps, unsigned char type );

    /*
     * Send a nak for the given packet type. Return 1 if the packet
     * sent OK, othewise -1.
     */
int gpsSendNak( GpsHandle gps, unsigned char type );

    /*
     * Receive a frame from the gps unit indicated by the GpsHandle.
     * Data is put into buf for up to *cnt bytes.  *cnt is updated
     * with the number of bytes actually received.  to specifies
     * a timeout before the start of a message is received.  Use -1
     * to block.
     *
     * Function returns:
     *	1 - data received
     *	0 - timeout
     *	-1 - error
     */
int gpsRecv( GpsHandle gps, int to, unsigned char * buf, int * cnt );

    /*
     * send a frame and wait for an ack/nak.  If nak'd re-send the frame.
     * Return 1 if all ok, -1 if frame can not be sent/is not acknowledged.
     */
int gpsSendWait( GpsHandle gps, const unsigned char * buf, int cnt );
