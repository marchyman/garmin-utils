/*
 *	$Id: gpsformat.c,v 1.1 1998/05/12 23:13:05 marc Exp $
 *
 *	Copyright (c) 1998 Marco S. Hyman
 *
 *	Permission to copy all or part of this material for any purpose is
 *	granted provided that the above copyright notice and this paragraph
 *	are duplicated in all copies.  THIS SOFTWARE IS PROVIDED ``AS IS''
 *	AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 *	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *	FOR A PARTICULAR PURPOSE.
 *
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gpsformat.h"

typedef enum {
    START,
    WAYPOINTS,
    ROUTES,
    TRACKS
} FormatState;

static FormatState
scanType( GpsHandle gps, unsigned char * buf )
{
    if ( *buf == '[' ) {
	if ( strncmp( buf, "[waypoints", 10 ) == 0 ) {
	    return WAYPOINTS;
	}
	if ( strncmp( buf, "[routes", 7 ) == 0 ) {
	    return ROUTES;
	}
	if ( strncmp( buf, "[tracks", 7 ) == 0 ) {
	    return TRACKS;
	}
    }
    return START;
}

    /*
     * Look for waypoints and/or route headers.  If found and in a
     * valid format build a gps packet and return it in a GpsListEntry.
     * Return 0 if a packet could not be created.
     */
static GpsListEntry *
scanWaypoint( GpsHandle gps, unsigned char * buf )
{
    GpsListEntry * entry = 0

    /* Assume it is a waypoint or route header and build the
       appropriate entry. */

    ;;;
    return entry;
}

    /*
     * Look for track entries.  If found and in a valid format build a gps
     * packet and return it in a GpsListEntry.  Return 0 if a packet could
     * not be created.
     */
static GpsListEntry *
scanTrack( GpsHandle gps, unsigned char * buf )
{
    GpsListEntry * entry = 0;

    /* Assume this entry is part of a track log
       appropriate entry. */

    ;;;
    return entry;
}

GpsLists *
gpsFormat( GpsHandle gps, FILE * stream )
{
    unsigned char buf[ GPS_BUF_LEN ];
    GpsLists * lists = 0;
    GpsLists * cur = 0;
    GpsListEntry * entry = 0;
    FormatState state = START;
    int ix;

    while ( fgets( buf, sizeof buf, stream ) ) {
	/* skip any leading whitespace */
	for ( ix = 0; buf[ ix ]; ix += 1 ) {
	    if ( ! isspace( buf[ ix ] ) ) {
		break;
	    }
	}

	/* Ignore comments and/or empty lines */
	if (( ! buf[ ix ] ) || ( buf[ ix ] == '#' )) {
	    continue;
	}

	/* check for list terminator */
	if (( buf[ ix ] == '[' ) && (strncmp( &buf[ ix ], "[end", 4 ) == 0 )) {
	    state = START;
	}

	switch ( state ) {
	  case START:
	    state = scanType( gps, &buf[ ix ] );
	    if ( state != START ) {
		if ( cur ) {
		    cur->next = (GpsLists *) malloc( sizeof( GpsLists ) );
		    assert( cur->next );
		    cur = cur->next;
		} else {
		    lists = (GpsLists *) malloc( sizeof( GpsLists ) );
		    assert( lists );
		    cur = lists;
		}
		cur->next = 0;
		cur->list = (GpsListHead *) malloc( sizeof( GpsListHead ) );
		assert( cur->list );
		cur->list->head = 0;
		cur->list->tail = 0;
		cur->list->count = 0;
	    }
	    continue;
	  case WAYPOINTS:
	  case ROUTES:
	    entry = scanWaypoint( gps, &buf[ ix ] );
	    break;
	  case TRACKS:
	    entry = scanTrack( gps, &buf[ ix ] );
	    break;
	}
	if ( entry ) {
	    if ( ! cur->list->head ) {
		cur->list->head = entry;
	    } else {
		cur->list->tail->next = entry;
	    }
	    cur->list->tail = entry;
	    cur->list->count += 1;
	}
    }
    if ( state != START ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "Missing [end transfer...] record, possible data loss" );
	}
    }

    return lists;
}
