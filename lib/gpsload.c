/*
 *	$snafu: gpsload.c,v 1.2 2001/05/02 00:34:54 marc Exp $
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

#include <stdio.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gps2.h"
#include "gpsformat.h"
#include "gpsload.h"

    /*
     * Send a start transfer.
     */
static int
startLoad( GpsHandle gps, int records )
{
    unsigned char buf[ 4 ];

    if ( gpsDebug( gps ) > 2 ) {
	warnx( "start: load" );
    }
    buf[ 0 ] = xfrBegin;
    buf[ 1 ] = (unsigned char) records;
    buf[ 2 ] = (unsigned char) ( records << 8 );
    return gpsSendWait( gps, buf, 3 );
}

static int
doLoad( GpsHandle gps, GpsListEntry * entry )
{
    while ( entry ) {
	if ( gpsSendWait( gps, entry->data, entry->dataLen ) != 1 ) {
	    return -1;
	}
	entry = entry->next;
    }
    return 1;
}

static int
endLoad( GpsHandle gps, int loadType )
{
    unsigned char buf[ 4 ];

    if ( gpsDebug( gps ) > 2 ) {
	warnx( "end: load" );
    }
    buf[ 0 ] = xfrEnd;
    buf[ 1 ] = (unsigned char) loadType;
    buf[ 2 ] = (unsigned char) ( loadType << 8 );
    return gpsSendWait( gps, buf, 3 );
}

static int
cancelLoad( GpsHandle gps )
{
    unsigned char buf[ 4 ];

    if ( gpsDebug( gps ) > 2 ) {
	warnx( "fail: load" );
    }
    buf[ 0 ] = xfrEnd;
    buf[ 1 ] = (unsigned char) CMD_ABORT_XFR;
    buf[ 2 ] = 0;
    return gpsSendWait( gps, buf, 3 );
}

int
gpsLoad( GpsHandle gps, GpsLists * lists )
{
    while ( lists ) {
	if ( startLoad( gps, lists->list->count ) != 1 ) {
	    return -1;
	}
	if ( doLoad( gps, lists->list->head ) != 1 ) {
	    cancelLoad( gps );
	    return -1;
	} else {
	    if ( endLoad( gps, lists->list->type ) != 1 ) {
		return -1;
	    }
	}
	lists = lists->next;
    }
    return 1;
}
