/*
 *	$snafu: gpsformat.c,v 1.11 2003/04/11 23:46:53 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpslib.h"

/*
 * decode state values
 */
enum {
	START,
	WAYPOINTS,
	ROUTES,
	TRACKS
};

/*
 * Figure out the operating state from the data in the buffer arg.
 */
static int
scan_state(u_char *buf)
{
	int state = START;
	if (*buf == '[') {
		if (strncmp(buf, RTE_HDR, sizeof RTE_HDR - 1) == 0)
			state =ROUTES;
		else if (strncmp(buf, TRK_HDR, sizeof TRK_HDR - 1) == 0)
			state = TRACKS;
		else if (strncmp(buf, WPT_HDR, sizeof WPT_HDR - 1) == 0)
			state = WAYPOINTS;
	}
	return state;
}

#if 0

/*
 * Build a list entry that contains the given data buffer.
 */
static struct gps_list_entry *
build_list_entry(u_char *data, int data_len)
{
	struct gps_list_entry *entry = malloc(sizeof(struct gps_list_entry));
	assert(entry != NULL);

	entry->next = 0;
	entry->data = data;
	entry->data_len = data_len;
	return entry;
}


    /*
     * Append data to a buffer. The rules are:
     *	1) data must be upper case
     *	2) If data less than reqd len null terminate.
     *	3) if data still less than reqd len then space pad.
     * Returns pointer to buffer after data has been added.
     */
static u_char *
addString( u_char * buf, u_char * data, int len )
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
doubleToSemicircle( double f, u_char * b )
{
    long work = f * ( 0x80000000 ) / 180.0;
    b[ 0 ] = (u_char) work;
    b[ 1 ] = (u_char) ( work >> 8 );
    b[ 2 ] = (u_char) ( work >> 16 );
    b[ 3 ] = (u_char) ( work >> 24 );
}

static GpsListEntry *
scanRoute( GpsHandle gps, u_char * buf )
{
    int route;			/* route number */
    char comment[ 24 ];		/* route comments */
    int result;			/* input scan results */
    u_char * data;	/* gps data buffer */
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
    data[ len++ ] = (u_char) route;

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
scanWaypoint( GpsHandle gps, u_char * buf, FormatState state )
{
    char name[ 8 ];		/* waypoint name */
    double lat;			/* latitude */
    double lon;			/* longitude */
    int sym;			/* symbol */
    int disp;			/* symbol display mode */
    char comment[ 44 ];		/* comment */
    int result;			/* input scan results */
    u_char * data;	/* gps data buffer */
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
        data[ len++ ] = (u_char) sym;

        /* byte 61: display option */
        data[ len++ ] = (u_char) disp;
    } else if (wptType == D104) {
        /* bytes 60-63: distance (uploaded as zero) */
        data[ len++ ] = 0;
        data[ len++ ] = 0;
        data[ len++ ] = 0;
        data[ len++ ] = 0;

        /* bytes 64-65: symbol */
        data[ len++ ] = (u_char) sym;
        data[ len++ ] = (u_char) (sym >> 8);

        /* byte 66: display option */
        data[ len++ ] = (u_char) disp;
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
scanTrack( GpsHandle gps, u_char * buf )
{
    double lat;			/* latitude */
    double lon;			/* longitude */
    u_char date[ 16 ];   /* track date (not used for upload) */
    u_char time[ 16 ];	/* track time (not used for upload) */
    u_char start[ 8 ];	/* start flag */
    int result;			/* input scan results */
    u_char * data;	/* gps data buffer */
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
    data[ len++ ] = (u_char) startFlag;

    return buildListEntry( data, len );
}

#endif


/*
 * create a new list
 */
static void
gps_list_new(struct gps_lists **lists, struct gps_lists **cur, int state)
{
	struct gps_lists *new;

	new = malloc(sizeof(struct gps_lists));
	assert(new != NULL);
	new->next = 0;
	new->list = malloc(sizeof(struct gps_list_head));
	assert( new->list );
	switch (state) {
	case WAYPOINTS:
		new->list->type = CMD_WPT;
		break;
	case ROUTES:
		new->list->type = CMD_RTE;
		break;
	case TRACKS:
		new->list->type = CMD_TRK;
		break;
	case START:
		break;
	}
	new->list->head = 0;
	new->list->tail = 0;
	new->list->count = 0;
	if (*cur) {
		(*cur)->next = new;
		(*cur) = new;
	} else
		(*cur) = (*lists) = new;
}

/*
 * Convert a given file, assumed to be in the same format output
 * by gpsprint, to lists of gps records ready to upload to a gps
 * unit.
 *
 * The lists and each item from the list come from the heap and
 * should be released.
 */
struct gps_lists *
gps_format(gps_handle gps, FILE *stream)
{
	u_char buf[GPS_BUF_LEN];
	struct gps_lists *lists = 0;
	struct gps_lists *cur = 0;
	struct gps_list_entry *entry = 0;
	int state = START;
	int ix;

	while (fgets(buf, sizeof buf, stream)) {

		/* skip any leading whitespace */
		for (ix = 0; buf[ix]; ix += 1)
			if (! isspace(buf[ix]))
				break;

		/* Ignore comments and/or empty lines */
		if (!buf[ix] || buf[ix] == '#' )
			continue;

		/* check for list terminator */
		if (buf[ix] == '[' && strncmp(&buf[ix], "[end", 4) == 0) {
			gps_printf(gps, 3, "...end\n");
			state = START;
		}

		/* process the content of the buffer according to
		   the current state */
		switch (state) {
		case START:
			state = scan_state(&buf[ix]);
			if (state != START) {
				gps_printf(gps, 3, __func__ ": processing %s\n",
					   &buf[ix]);
				gps_list_new(&lists, &cur, state);
			}
			continue;
		case WAYPOINTS:
			/* entry = scanWaypoint( gps, &buf[ ix ], state ); */
			break;
		case ROUTES:
#if 0
			if ( buf[ ix ] == '*' ) {
				entry = scanRoute( gps, &buf[ ix ] );
			} else {
				entry = scanWaypoint( gps, &buf[ ix ], state );
			}
#endif
			break;
		case TRACKS:
			/* entry = scanTrack( gps, &buf[ ix ] ); */
			break;
		}

		/* If entry is not null link it into the current list */
		if (entry != NULL) {
			if (cur->list->head == NULL)
				cur->list->head = entry;
			else
				cur->list->tail->next = entry;
			cur->list->tail = entry;
			cur->list->count += 1;
		}
	}
	return lists;
}
