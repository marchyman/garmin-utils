/*
 *	$snafu: gpsformat.c,v 1.18 2003/04/17 23:20:13 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpslib.h"

/*
 * decode states
 */
enum {
	START,
	WAYPOINTS,
	ROUTES,
	TRACKS
};

/*
 * Magic value that translates to a float value of 1.0e25 in the GPS
 * and indicates an unsupported or unknown value.
 */
static u_char no_val[] = { 81, 89, 4, 105 };

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

static u_char *
gps_buffer_new(void)
{
	u_char *data;

	data = malloc(GPS_FRAME_MAX);
	assert(data != NULL);
	return data;
}

/*
 * convert the given double to a garmin `semicircle' and stuff
 * it in to b (assumed to be at least 4 characters wide) in
 * the garmin (little endian) order.
 */
static void
double2semicircle(double f, u_char *b)
{
	long work = f * 0x80000000 / 180.0;
	b[0] = (u_char) work;
	b[1] = (u_char) (work >> 8);
	b[2] = (u_char) (work >> 16);
	b[3] = (u_char) (work >> 24);
}

/*
 * Append data to a buffer. The rules are:
 *	1) data inserted is upper case
 *	2) newline is converted to null
 *	3) If data len less than buffer len null terminate.
 *	4) pad remainder of buffer with spaces
 * Returns pointer to buffer after data has been added.
 */
static u_char *
add_string(u_char *buf, u_char *data, int len)
{
	while ( len-- ) {
		if (*data == '\n')
			*data = 0;
		if (*data)
			*buf++ = toupper(*data++);
		else {
			*buf++ = 0;
			if (len) {
				memset(buf, ' ', len);
				buf += len;
				len = 0;
			}
		}
	}
	return buf;
}


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
 * build the data common to most waypoints, returning a pointer to
 * the data buffer and an updated lenght
 */
static u_char *
wpt_common(int *datalen, int state, char *name, double lat, double lon,
	   char *cmnt)
{
	u_char *data;
	int len;
	int ix;

	data = gps_buffer_new();
	len = 0;

	/* byte 0: data_type */
	data[len++] = state == WAYPOINTS ? p_wpt_data : p_rte_wpt_data;

	/* byte 1-6: waypoint name */
	for (ix = 0; ix < 6; ix += 1) {
		data[len++] = toupper(name[ix]);
	}

	/* byte 7-10: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 11-14: longitute */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 15-18: zero */
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;

	/* byte 19-58: comment */
	add_string(&data[len], cmnt, 40);
	len += 40;

	*datalen = len;
	return data;
}

static struct gps_list_entry *
d100_wpt(int state, char *name, double lat, double lon, char *cmnt)
{
	u_char *data;
	int len;

	data = wpt_common(&len, state, name, lat, lon, cmnt);
	return build_list_entry(data, len);
}

static struct gps_list_entry *
d101_wpt(int state, char *name, double lat, double lon, char *cmnt, int sym)
{
	u_char *data;
	int len;

	data = wpt_common(&len, state, name, lat, lon, cmnt);

	/* byte 59-62: proximity (float) -- set to zero */
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	
	/* byte 63: symbol */
	data[len++] = (u_char) sym;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d102_wpt(int state, char *name, double lat, double lon, char *cmnt, int sym)
{
	u_char *data;
	int len;

	data = wpt_common(&len, state, name, lat, lon, cmnt);

	/* byte 59-62: proximity (float) -- set to zero */
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	
	/* byte 63-64: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d103_wpt(int state, char *name, double lat, double lon, char *cmnt, int sym,
	 int disp)
{
	u_char *data;
	int len;

	data = wpt_common(&len, state, name, lat, lon, cmnt);

	/* byte 59: symbol */
	data[len++] = (u_char) sym;

	/* byte 60: display */
	data[len++] = (u_char) disp;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d104_wpt(int state, char *name, double lat, double lon, char *cmnt, int sym,
	 int disp)
{
	u_char *data;
	int len;

	data = wpt_common(&len, state, name, lat, lon, cmnt);

	/* byte 59-60: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);

	/* byte 61: display */
	data[len++] = (u_char) disp;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d105_wpt(int state, double lat, double lon, char *cmnt, int sym)
{
	u_char *data;
	int len;

	data = gps_buffer_new();
	len = 0;

	/* byte 0: data_type */
	data[len++] = state == WAYPOINTS ? p_wpt_data : p_rte_wpt_data;

	/* byte 1-4: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 5-8: longitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 9-10: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);

	/* byte 11-50: ident (saved as comment by print code) */
	add_string(&data[len], cmnt, 40);
	len += 40;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d106_wpt(int state, double lat, double lon, char *cmnt, int sym)
{
	u_char *data;
	int len;

	data = gps_buffer_new();
	len = 0;

	/* byte 0: data type */
	data[len++] = state == WAYPOINTS ? p_wpt_data : p_rte_wpt_data;

	/* byte 1-14: class/subclass.  0 == user waypoint */
	memset(&data[len], 0, 14);
	len += 14;

	/* byte 15-18: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 19-22: longitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 23-24: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);

	/* byte 25-64: ident (saved as comment by print code) */
	add_string(&data[len], cmnt, 40);
	len += 40;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d107_wpt(int state, char *name, double lat, double lon, char *cmnt, int sym,
	 int disp)
{
	u_char *data;
	int len;

	data = wpt_common(&len, state, name, lat, lon, cmnt);

	/* byte 59: symbol */
	data[len++] = (u_char) sym;

	/* byte 60: display */
	data[len++] = (u_char) disp;

	/* byte 61-64: proximity */
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;

	/* byte 65: color (0 == default) */
	data[len++] = 0;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d108_wpt(int state, double lat, double lon, char *cmnt, int sym, int disp)
{
	u_char *data;
	int len;
	int tlen;

	data = gps_buffer_new();
	len = 0;

	/* byte 0: data type */
	data[len++] = state == WAYPOINTS ? p_wpt_data : p_rte_wpt_data;

	/* byte 1: waypoint class (0 == user) */
	data[len++] = 0;

	/* byte 2: waypoint color (0xff == default) */
	data[len++] = (u_char) 0xff;

	/* byte 3: display */
	data[len++] = (u_char) disp;

	/* byte 4: attributes (0x60 per garmin doc) */
	data[len++] = (u_char) 0x60;

	/* byte 5-6: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);
	
	/* byte 7-24: subclass (value per garmin doc) */
	memset(&data[len], 0, 6);
	len += 6;
	memset(&data[len], 0xff, 12);
	len += 12;

	/* byte 25-28: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 29-32: longitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 33-36: alt */
	memcpy(&data[len], no_val, 4);
	len += 4;

	/* byte 37-40: depth */
	memcpy(&data[len], no_val, 4);
	len += 4;

	/* byte 41-44: proximity */
	memcpy(&data[len], no_val, 4);
	len += 4;

	/* byte 45-48: state and country codes: set to the empty string */
	data[len++] = ' ';
	data[len++] = ' ';
	data[len++] = ' ';
	data[len++] = ' ';

	/* byte 49-99: ident (max 51 characters) */
	tlen = strlcpy(&data[len], cmnt, 51);
	if (++tlen > 51)
		tlen = 51;
	len += tlen;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d109_wpt(int state, double lat, double lon, char *cmnt, int sym,
	 int disp)
{
	u_char *data;
	int len;
	int tlen;

	data = gps_buffer_new();
	len = 0;

	/* byte 0: data type */
	data[len++] = state == WAYPOINTS ? p_wpt_data : p_rte_wpt_data;

	/* byte 1: data packet type (1 from the garmin doc) */
	data[len++] = 1;

	/* byte 2: waypoint class */
	data[len++] = 0;

	/* byte 3: display and color (1f == default color */
	data[len++] = ((disp << 5) & 0x60) | 0x1f;

	/* byte 4: attributes (0x70 per garmin doc) */
	data[len++] = (u_char) 0x70;

	/* byte 5-6: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);
	
	/* byte 7-24: subclass (assumes value same as D108) */
	memset(&data[len], 0, 6);
	len += 6;
	memset(&data[len], 0xff, 12);
	len += 12;

	/* byte 25-28: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 29-32: longitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 33-36: alt */
	memcpy(&data[len], no_val, 4);
	len += 4;

	/* byte 37-40: depth */
	memcpy(&data[len], no_val, 4);
	len += 4;

	/* byte 41-44: proximity */
	memcpy(&data[len], no_val, 4);
	len += 4;

	/* byte 45-48: state and country codes: set to the empty string */
	data[len++] = ' ';
	data[len++] = ' ';
	data[len++] = ' ';
	data[len++] = ' ';

	/* byte 49-52: link ete, default = 0xffffffff */
	data[len++] = (u_char) 0xff;
	data[len++] = (u_char) 0xff;
	data[len++] = (u_char) 0xff;
	data[len++] = (u_char) 0xff;

	/* byte 53-103: ident (max 51 characters) */
	tlen = strlcpy(&data[len], cmnt, 51);
	if (++tlen > 51)
		tlen = 51;
	len += tlen;

	/* Add null bytes for the empty fields comment, facility, city
	   address, and cross road */
	data[len++] = 0;	/* comment */
	data[len++] = 0;	/* facility */
	data[len++] = 0;	/* city */
	data[len++] = 0;	/* address */
	data[len++] = 0;	/* cross road */

	return build_list_entry(data, len);
}

/*
 * decode the buffer as a waypoint and format according to the required
 * waypoint type.   Data is expected to be in this format
 *
 *	name lat long sym/disp comments
 */
static struct gps_list_entry *
waypoints(gps_handle gps, u_char *buf, int state)
{
	struct gps_list_entry *entry = 0;
	int result;
	char name[8];		/* waypoint name */
	double lat;		/* latitude */
	double lon;		/* longitude */
	int sym;		/* symbol */
	int disp;		/* symbol display mode */
	char comment[44];	/* comment */
	int wpt;

	/* break the input data up into its component fields */
	result = sscanf(buf, "%6c %lf %lf %*f %d/%d %40c",
			name, &lat, &lon, &sym, &disp, comment);
	if (result != 6) {
		result = sscanf(buf, "%6c %lf %lf %d/%d %40c",
				name, &lat, &lon, &sym, &disp, comment);
		if (result != 6) {
			gps_printf(gps, 1, __func__ ": bad format: %s\n", buf);
			return 0;
		}
	}
	name[6] = 0;
	comment[40] = 0;
	gps_printf(gps, 3, "wpt: %s %lf %lf %d/%d %s\n", name, lat, lon, sym,
		   disp, comment);

	/* Now figure out which waypoint format is being used and
	   call the appropriate routine */

	switch (state) {
	case WAYPOINTS:
		wpt = gps_get_wpt_type(gps);
		break;
	case ROUTES:
		wpt = gps_get_rte_wpt_type(gps);
		break;
	default:
		wpt = 0;
	}
	switch (wpt) {
	case D100:
		entry = d100_wpt(state, name, lat, lon, comment);
		break;
	case D101:
		entry = d101_wpt(state, name, lat, lon, comment, sym);
		break;
	case D102:
		entry = d102_wpt(state, name, lat, lon, comment, sym);
		break;
	case D103:
		entry = d103_wpt(state, name, lat, lon, comment, sym, disp);
		break;
	case D104:
		entry = d104_wpt(state, name, lat, lon, comment, sym, disp);
		break;
	case D105:
		entry = d105_wpt(state, lat, lon, comment, sym);
		break;
	case D106:
		entry = d106_wpt(state, lat, lon, comment, sym);
		break;
	case D107:
		entry = d107_wpt(state, name, lat, lon, comment, sym, disp);
		break;
	case D108:
		entry = d108_wpt(state, lat, lon, comment, sym, disp);
		break;
	case D109:
		entry = d109_wpt(state, lat, lon, comment, sym, disp);
		break;
	default:
		gps_printf(gps, 1, "unknown waypoint type %d\n", wpt);
		break;
	}

	return entry;
}

#if 0

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
		if (buf[ix] == 0 || buf[ix] == '#')
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
			entry = waypoints(gps, &buf[ix], state);
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
