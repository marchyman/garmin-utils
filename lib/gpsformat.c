/*
 *	$snafu: gpsformat.c,v 1.7 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
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
	    if ( gpsDebug( gps ) > 1 ) {
		warnx( "waypoints..." );
	    }
	    return WAYPOINTS;
	}
	if ( strncmp( buf, "[routes", 7 ) == 0 ) {
	    if ( gpsDebug( gps ) > 1 ) {
		warnx( "routes..." );
	    }
	    return ROUTES;
	}
	if ( strncmp( buf, "[tracks", 7 ) == 0 ) {
	    if ( gpsDebug( gps ) > 1 ) {
		warnx( "tracks..." );
	    }
	    return TRACKS;
	}
    }
    return START;
}

    /*
     * Build a list entry that contains the given data buffer.
     */
static GpsListEntry *
buildListEntry( unsigned char * data, int dataLen )
{
    GpsListEntry * entry = (GpsListEntry *) malloc( sizeof( GpsListEntry ) );

    assert( entry );
    entry->next = 0;
    entry->data = data;
    entry->dataLen = dataLen;
    return entry;
}

    /*
     * Append data to a buffer. The rules are:
     *	1) data must be upper case
     *	2) If data less than reqd len null terminate.
     *	3) if data still less than reqd len then space pad.
     * Returns pointer to buffer after data has been added.
     */
static unsigned char *
addString( unsigned char * buf, unsigned char * data, int len )
{
    while ( len-- ) {
	if ( *data == '\n' ) {
	    *data = 0;
	}
	if ( *data ) {
	    *buf++ = toupper( *data++ );
	} else {
	    *buf++ = 0;
	    if ( len ) {
		memset( buf, ' ', len );
		buf += len;
		len = 0;
	    }
	}
    }
    return buf;
}

    /*
     * convert the given double to a garmin `semicircle' and stuff
     * it in to b (assumed to be at least 4 characters wide) in
     * the garmin (little endian) order.
     */
static void
doubleToSemicircle( double f, unsigned char * b )
{
    long work = f * ( 0x80000000 ) / 180.0;
    b[ 0 ] = (unsigned char) work;
    b[ 1 ] = (unsigned char) ( work >> 8 );
    b[ 2 ] = (unsigned char) ( work >> 16 );
    b[ 3 ] = (unsigned char) ( work >> 24 );
}

static GpsListEntry *
scanRoute( GpsHandle gps, unsigned char * buf )
{
    int route;			/* route number */
    char comment[ 24 ];		/* route comments */
    int result;			/* input scan results */
    unsigned char * data;	/* gps data buffer */
    int len;			/* gps data len */

    /* break the input data up into its component fields */
    result = sscanf( buf, "**%d %20c", &route, comment );
    if ( result != 2 ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "bad route header: %s", buf );
	}
	return 0;
    }
    comment[ 20 ] = 0;

    if ( gpsDebug( gps ) > 1 ) {
	warnx( "route %d", route );
    }

    /* byte 0: route header type */
    data = malloc( GPS_FRAME_MAX );
    assert( data );
    len = 0;
    data[ len++ ] = rteHdr;

    /* byte 1: route number */
    data[ len++ ] = (unsigned char) route;

    /* byte 2-22: route comment */
    addString( &data[ len ], comment, 20 );
    len += 20;

    return buildListEntry( data, len );
}

    /*
     * Look for waypoints and/or route headers.  If found and in a
     * valid format build a gps packet and return it in a GpsListEntry.
     * Return 0 if a packet could not be created.
     */
static GpsListEntry *
scanWaypoint( GpsHandle gps, unsigned char * buf, FormatState state )
{
    char name[ 8 ];		/* waypoint name */
    double lat;			/* latitude */
    double lon;			/* longitude */
    int sym;			/* symbol */
    int disp;			/* symbol display mode */
    char comment[ 44 ];		/* comment */
    int result;			/* input scan results */
    unsigned char * data;	/* gps data buffer */
    int len;			/* gps data len */
    int ix;			/* general use index */
    int wptType;
    
    /* break the input data up into its component fields */
    result = sscanf( buf, "%6c %lf %lf %d/%d %40c",
		    name, &lat, &lon, &sym, &disp, comment );
    if ( result!= 6 ) {
	if ( gpsDebug( gps ) ) {
	    warnx( "bad waypoint: %s", buf );
	}
	return 0;
    }
    name[ 6 ] = 0;
    comment[ 40 ] = 0;

    if ( gpsDebug( gps ) > 1 ) {
	warnx( "waypoint %s", name );
    }

    wptType = gpsGetWptType (gps);
    
    /* byte 1: waypoint type */
    data = malloc( GPS_FRAME_MAX );
    assert( data );
    len = 0;
    if ( state == ROUTES ) {
	data[ len++ ] = rteWptData;
    } else {
	data[ len++ ] = wptData;
    }

    /* bytes 2-7: waypoint name */
    for ( ix = 0; ix < 6; ix += 1 ) {
	data[ len++ ] = toupper( name[ ix ] );
    }

    /* bytes 8-11: latitude */
    doubleToSemicircle( lat, &data[ len ] );
    len += 4;

    /* bytes 12-15: longitude */
    doubleToSemicircle( lon, &data[ len ] );
    len += 4;

    /* bytes 16-19: zero */
    data[ len++ ] = 0;
    data[ len++ ] = 0;
    data[ len++ ] = 0;
    data[ len++ ] = 0;

    /* bytes 20-59: comments */
    addString( &data[ len ], comment, 40 );
    len += 40;

    if (wptType == D100) {
        /* do nothing */
    } else if (wptType == D103) {
        /* byte 60: symbol */
        data[ len++ ] = (unsigned char) sym;

        /* byte 61: display option */
        data[ len++ ] = (unsigned char) disp;
    } else if (wptType == D104) {
        /* bytes 60-63: distance (uploaded as zero) */
        data[ len++ ] = 0;
        data[ len++ ] = 0;
        data[ len++ ] = 0;
        data[ len++ ] = 0;

        /* bytes 64-65: symbol */
        data[ len++ ] = (unsigned char) sym;
        data[ len++ ] = (unsigned char) (sym >> 8);

        /* byte 66: display option */
        data[ len++ ] = (unsigned char) disp;
    } else {
        errx (1, "unsupported device");
    }

    return buildListEntry( data, len );
}

    /*
     * Look for track entries.  If found and in a valid format build a gps
     * packet and return it in a GpsListEntry.  Return 0 if a packet could
     * not be created.
     */
static GpsListEntry *
scanTrack( GpsHandle gps, unsigned char * buf )
{
    double lat;			/* latitude */
    double lon;			/* longitude */
    unsigned char date[ 16 ];   /* track date (not used for upload) */
    unsigned char time[ 16 ];	/* track time (not used for upload) */
    unsigned char start[ 8 ];	/* start flag */
    int result;			/* input scan results */
    unsigned char * data;	/* gps data buffer */
    int len;			/* gps data len */

    int startFlag = 0;		/* start of track */

    /* break the input data up into its component fields */
    result = sscanf( buf, "%lf %lf %15s %15s %7s", &lat, &lon, date, time, start );
    switch ( result ) {
      case 2:
      case 3:
      case 4:
	break;
      case 5:
	if ( strcmp( start, "start" ) == 0 ) {
	    startFlag = 1;
	    break;
	}
	/* fall through */
      default:
	if ( gpsDebug( gps ) ) {
            printf ("%d\n", result);
	    warnx( "bad track: %s", buf );
	}
	return 0;
    }

    /* byte 1: waypoint type */
    data = malloc( GPS_FRAME_MAX );
    assert( data );
    len = 0;
    data[ len++ ] = trkData;

    /* byte 2-5: latitude */
    doubleToSemicircle( lat, &data[ len ] );
    len += 4;

    /* byte 6-9: longitude */
    doubleToSemicircle( lon, &data[ len ] );
    len += 4;

    /* bytes 10-13: time (uploaded as zero */
    data[ len++ ] = 0;
    data[ len++ ] = 0;
    data[ len++ ] = 0;
    data[ len++ ] = 0;

    /* byte 14: start indicator */
    data[ len++ ] = (unsigned char) startFlag;

    return buildListEntry( data, len );
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
	    if ( gpsDebug( gps ) > 1 ) {
		warnx( "...end" );
	    }
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
		switch ( state ) {
		  case WAYPOINTS:
		    cur->list->type = CMD_WPT;
		    break;
		  case ROUTES:
		    cur->list->type = CMD_RTE;
		    break;
		  case TRACKS:
		    cur->list->type = CMD_TRK;
		    break;
		  case START:
		    break;
		}
		cur->list->head = 0;
		cur->list->tail = 0;
		cur->list->count = 0;
	    }
	    continue;
	  case WAYPOINTS:
	    entry = scanWaypoint( gps, &buf[ ix ], state );
	    break;
	  case ROUTES:
	    if ( buf[ ix ] == '*' ) {
		entry = scanRoute( gps, &buf[ ix ] );
	    } else {
		entry = scanWaypoint( gps, &buf[ ix ], state );
	    }
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
