/*
 *	$snafu: gpsproto.h,v 1.6 2001/06/19 04:36:47 marc Exp $
 *
 *	Copyright (c) 1998 Marco S. Hyman
 *
 *	Permission to copy all or part of this material with or without
 *	modification for any purpose is granted provided that the above
 *	copyright notice and this paragraph are duplicated in all copies.
 *
 *	THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

    /*
     * Known GPS protocol identifiers
     */
#define etx		3
#define ack		6
#define cmdType		10
#define xfrEnd		12
#define utcData		14
#define dle		16
#define nak		21
#define xfrBegin	27
#define rteHdr		29
#define rteWptData	30
#define trkData		34
#define wptData		35
#define protoCap	253
#define prodRqst	254
#define prodResp	255

    /*
     * Raw input buffer -- read up to this many characters at a time
     */
#define GPS_BUF_LEN 512

    /*
     * Size of an application data buffer.  This is large enough to
     * hold the frame type plus the largest allowable frame.
     */
#define GPS_FRAME_MAX	256

    /*
     * Gps command (upload/download) types.
     */
typedef enum {
    CMD_ABORT_XFR = 0,
    CMD_RTE = 4,
    CMD_UTC = 5,
    CMD_TRK = 6,
    CMD_WPT = 7
} GpsCmdId;

    /*
     * Several GPS protocols
     */
#define	A100		100	/* waypoint transfer protocol */

    /*
     * Known GPS waypoint packet types
     */
#define D100		100
#define D103		103
#define D104		104
