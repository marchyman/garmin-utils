/*
 *	$Id: gpsproto.h,v 1.2 1998/05/13 04:03:29 marc Exp $
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
     * Known GPS protocol identifiers
     */
#define etx		3
#define ack		6
#define cmdType		10
#define xfrEnd		12
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

