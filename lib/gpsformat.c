/*
 * $snafu: gpsformat.c,v 1.26 2003/06/12 16:45:00 marc Exp $
 *
 * Public Domain, 2001, Marco S Hyman <marc@snafu.org>
 */

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpslib.h"

#define GPS_STRING_MAX	51

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
			state = ROUTES;
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
 * convert len bytes of buf in hex representation to binary
 */
static void
gps_get_info(char *info, char *buf, int len)
{
	int ix;
	int val;

	for (ix = 0; ix < len; ix += 2) {
		sscanf(&buf[ix], "%2x", &val);
		info[ix / 2] = (char) val;
	}
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
 *	1) newline is converted to null
 *	2) If data len less than buffer len null terminate.
 *	3) pad remainder of buffer with spaces
 * Returns pointer to buffer after data has been added.
 */
static u_char *
add_string(u_char *buf, u_char *data, int len)
{
	while ( len-- ) {
		if (*data == '\n')
			*data = 0;
		if (*data)
			*buf++ = *data++;
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
	double2semicircle(lon, &data[len]);
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
d108_wpt(int state, char *name, double lat, double lon, char *cmnt, int sym,
	 int disp, char *info)
{
	u_char *data;
	int len;
	int tlen;

	data = gps_buffer_new();
	len = 0;

	/* byte 0: data type */
	data[len++] = state == WAYPOINTS ? p_wpt_data : p_rte_wpt_data;

	/* byte 1: waypoint class (0 == user) */
	data[len++] = info[0];

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
	if (info[0] != 0) {
		memcpy(&data[len], &info[1], 18);
		len += 18;
	} else {
		memset(&data[len], 0, 6);
		len += 6;
		memset(&data[len], 0xff, 12);
		len += 12;
	}

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
	tlen = strlcpy(&data[len], name, GPS_STRING_MAX);
	if (++tlen > GPS_STRING_MAX)
		tlen = GPS_STRING_MAX;
	len += tlen;

	/* comment follows name (max 51 characters) */
	tlen = strlcpy(&data[len], cmnt, GPS_STRING_MAX);
	if (++tlen > GPS_STRING_MAX)
		tlen = GPS_STRING_MAX;
	len += tlen;

	data[len++] = 0;	/* facility */
	data[len++] = 0;	/* city */
	data[len++] = 0;	/* address */
	data[len++] = 0;	/* cross road */

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d109_wpt(int state, char *name, double lat, double lon, char *cmnt,
	 int sym, int disp, char *info)
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
	data[len++] = info[0];

	/* byte 3: display and color (1f == default color */
	data[len++] = ((disp << 5) & 0x60) | 0x1f;

	/* byte 4: attributes (0x70 per garmin doc) */
	data[len++] = (u_char) 0x70;

	/* byte 5-6: symbol */
	data[len++] = (u_char) sym;
	data[len++] = (u_char) (sym >> 8);
	
	/* byte 7-24: subclass (assumes value same as D108) */
	if (info[0] != 0) {
		memcpy(&data[len], &info[1], 18);
		len += 18;
	} else {
		memset(&data[len], 0, 6);
		len += 6;
		memset(&data[len], 0xff, 12);
		len += 12;
	}

	/* byte 25-28: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 29-32: longitude */
	double2semicircle(lon, &data[len]);
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

	/* byte 53-103: ident (max GPS_STRING_MAX characters) */
	tlen = strlcpy(&data[len], name, GPS_STRING_MAX);
	if (++tlen > GPS_STRING_MAX)
		tlen = GPS_STRING_MAX;
	len += tlen;

	/* comment follows name (max 51 characters) */
	tlen = strlcpy(&data[len], cmnt, GPS_STRING_MAX);
	if (++tlen > GPS_STRING_MAX)
		tlen = GPS_STRING_MAX;
	len += tlen;


	/* Add null bytes for the empty fields comment, facility, city
	   address, and cross road */
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
 *  lat long [A:altitude] [S:symbol] [D:display] [I:ident] [C:comment] [L:link]
 */
static struct gps_list_entry *
waypoints(gps_handle gps, u_char *buf, int state, int *link)
{
	struct gps_list_entry *entry = 0;
	double lat;			/* latitude */
	double lon;			/* longitude */
	int sym;			/* symbol */
	int disp;			/* symbol display mode */
	char name[GPS_STRING_MAX + 1];	/* waypoint name */
	char cmnt[GPS_STRING_MAX + 1];	/* comment */
	char data[GPS_STRING_MAX + 1];	/* waypoint class and subclass */

	char *beg;
	char *end;
	int len;
	int wpt;

	*link = -1;
	sym = disp = 0;
	name[0] = 0;
	cmnt[0] = 0;
	data[0] = 0;

	/* Latitude and longitude */
	sscanf(buf, "%lf %lf", &lat, &lon);

	/* key:value pairs */
	for (beg = strchr(buf, ':'); beg; beg = end) {
		end = strchr(beg + 1, ':');
		/* len includes space for null */
		if (end == NULL)
			len = strlen(beg + 1) + 1;
		else
			len = end - beg - 2;
		if (len > GPS_STRING_MAX)
			len = GPS_STRING_MAX;
		switch (toupper(beg[-1])) {
		case 'A':
			/* altitude ignored */
			break;
		case 'W':
			/* waypoint data (class and subclass) */
			gps_get_info(data, &beg[1], len);
			break;
		case 'S':
			/* symbol */
			sscanf(&beg[1], "%d", &sym);
			break;
		case 'D':
			/* display mode */
			sscanf(&beg[1], "%d", &disp);
			break;
		case 'I':
			strlcpy(name, &beg[1], len);
			break;
		case 'C':
			strlcpy(cmnt, &beg[1], len);
			break;
		case 'L':
			/* route link code */
			sscanf(&beg[1], "%d", link);
			break;
		default:
			gps_printf(gps, 1, __func__ ": unknown field ->%s\n",
				   &beg[-1]);
			continue;
		}
	}

	gps_printf(gps, 3, "wpt %f %f %d %d %s %s %d\n", lat, lon, sym,
		   disp, name, cmnt, *link);

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
		entry = d100_wpt(state, name, lat, lon, cmnt);
		break;
	case D101:
		entry = d101_wpt(state, name, lat, lon, cmnt, sym);
		break;
	case D102:
		entry = d102_wpt(state, name, lat, lon, cmnt, sym);
		break;
	case D103:
		entry = d103_wpt(state, name, lat, lon, cmnt, sym, disp);
		break;
	case D104:
		entry = d104_wpt(state, name, lat, lon, cmnt, sym, disp);
		break;
	case D105:
		entry = d105_wpt(state, lat, lon, cmnt, sym);
		break;
	case D106:
		entry = d106_wpt(state, lat, lon, cmnt, sym);
		break;
	case D107:
		entry = d107_wpt(state, name, lat, lon, cmnt, sym, disp);
		break;
	case D108:
		entry = d108_wpt(state, name, lat, lon, cmnt, sym, disp, data);
		break;
	case D109:
		entry = d109_wpt(state, name, lat, lon, cmnt, sym, disp, data);
		break;
	default:
		gps_printf(gps, 1, "unknown waypoint type %d\n", wpt);
		break;
	}
	return entry;
}

static struct gps_list_entry *
d200_route(int num)
{
	u_char *data;
	int len;

	data = gps_buffer_new();
	len = 0;

	data[len++] = p_rte_hdr;
	data[len++] = num;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d201_route(int num, char *cmnt)
{
	u_char *data;
	int len;

	data = gps_buffer_new();
	len = 0;

	data[len++] = p_rte_hdr;
	data[len++] = num;
	add_string(&data[len], cmnt, 20);
	len += 20;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
d202_route(char *cmnt)
{
	u_char *data;
	int len;
	int tlen;

	data = gps_buffer_new();
	len = 0;

	data[len++] = p_rte_hdr;
	tlen = strlcpy(&data[len], cmnt, GPS_STRING_MAX);
	if (++tlen > GPS_STRING_MAX)
		tlen = GPS_STRING_MAX;
	len += tlen;

	return build_list_entry(data, len);
}

/*
 * decode the buffer as a route header.  Route waypoints are handled by the
 * above waypoint code.   Data is expected to be in this format
 *
 *	**number name
 */
static struct gps_list_entry *
routes(gps_handle gps, u_char *buf)
{
	struct gps_list_entry *entry;
	char *p;
	int rte;
	int num;
	char cmnt[GPS_STRING_MAX + 1];

	sscanf(buf, "**%d", &num);
	p = strchr(buf, ' ');
	if (p)
		strlcpy(cmnt, p + 1, GPS_STRING_MAX);
	else
		cmnt[GPS_STRING_MAX] = 0;
	gps_printf(gps, 3, "route %d %s\n", num, cmnt);

	rte = gps_get_rte_hdr_type(gps);
	switch (rte) {
	case D200:
		entry = d200_route(num);
		break;
	case D201:
		entry = d201_route(num, cmnt);
		break;
	case D202:
		entry = d202_route(cmnt);
		break;
	default:
		entry = NULL;
		gps_printf(gps, 1, "unknown route hdr type %d\n", rte);
		break;
	}

	return entry;
}

static struct gps_list_entry *
route_link(gps_handle gps, int link)
{
	u_char *data;
	int len;

	data = gps_buffer_new();
	len = 0;

	data[len++] = p_rte_link;

	/* byte 1-2: class */
	data[len++] = (u_char) link;
	data[len++] = (u_char) (link >> 8);

	/* byte 3-20: subclass (value per garmin doc) */
	memset(&data[len], 0, 6);
	len += 6;
	memset(&data[len], 0xff, 12);
	len += 12;

	/* byte 21-??: ident (empty) */

	data[len++] = 0;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
track_hdr(gps_handle gps, u_char *buf)
{
	u_char *data;
	int len;
	int tlen;
	char name[GPS_STRING_MAX + 1];	/* track name */

	/* skip any leading whitespace and extract the name */
	for (len = 0; buf[len]; len += 1)
		if (! isspace(buf[len]))
			break;
	strlcpy(name, &buf[len], GPS_STRING_MAX);

	data = gps_buffer_new();
	len = 0;

	data[len++] = p_trk_hdr;

	/* byte 1: display */
	data[len++] = 1;

	/* byte 2: default color */
	data[len++] = 0xff;

	/* byte 3-n: ident (max GPS_STRING_MAX characters) */
	tlen = strlcpy(&data[len], name, GPS_STRING_MAX);
	if (++tlen > GPS_STRING_MAX)
		tlen = GPS_STRING_MAX;
	len += tlen;

	return build_list_entry(data, len);
}

static struct gps_list_entry *
tracks(gps_handle gps, u_char *buf)
{
	char *p;
	u_char *data;
	int len;
	int start;
	double lat;
	double lon;

	/*
	 * if the buffer starts with a date/time, skip them.
	 * The input should look like yyyy-mm-dd hh:mm:ss ...
	 */
	if (buf[4] == '-' && buf[7] == '-' && buf[13] == ':' && buf[16] == ':')
		buf += 19;

	/* Latitude and longitude */
	sscanf(buf, "%lf %lf", &lat, &lon);

	/* look for start flag */
	p = strrchr(buf, ' ');
	if (p != NULL)
		start = strcmp(p+1, "start") == 0;
	else
		start = 0;

	gps_printf(gps, 3, "trk %f %f%s\n", lat, lon, start ? " start" : "");

	data = gps_buffer_new();
	len = 0;

	data[len++] = p_trk_data;

	/* byte 1-4: latitude */
	double2semicircle(lat, &data[len]);
	len += 4;

	/* byte 5-8: longitude */
	double2semicircle(lon, &data[len]);
	len += 4;

	/* time (uploaded as zero) */
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;
	data[len++] = 0;

	/*
	 * load the magic "unknown" value for altitude and depth if
	 * supported by this unit.
	 */
	if (gps_get_trk_type(gps) == D301) {
		memcpy(&data[len], no_val, 4);
		len += 4;
		memcpy(&data[len], no_val, 4);
		len += 4;
	}

	/* start indicator */
	data[len++] = start;

	return build_list_entry(data, len);
}

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
 * Link the entry arg to the end of the list identified by the
 * cur arg.   Bump the list count.
 */
static void
gps_append_list(struct gps_lists *cur, struct gps_list_entry *entry)
{
	if (cur->list->head == NULL)
		cur->list->head = entry;
	else
		cur->list->tail->next = entry;
	cur->list->tail = entry;
	cur->list->count += 1;
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
	int link;
	int rte;
	char *p;

	while (fgets(buf, sizeof buf, stream)) {

		/* kill any trailing newline */
		if ((p = strrchr(buf, '\n')) != NULL)
			*p = 0;

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
			entry = waypoints(gps, &buf[ix], state, &link);
			break;
		case ROUTES:
			if (buf[ix] == '*')
				entry = routes(gps, &buf[ix]);
			else {
				entry = waypoints(gps, &buf[ix], state, &link);
				if (link != -1) {
					if (entry != NULL) {
						gps_append_list(cur, entry);
						entry = NULL;
					}
					rte = gps_get_rte_lnk_type(gps);
					if (rte == D210)
						entry = route_link(gps, link);
				}
			}
			break;
		case TRACKS:
			if (strncmp(&buf[ix], "Track:", 6) == 0) {
				if (gps_get_trk_hdr_type(gps) != 0)
					entry = track_hdr(gps, &buf[ix + 6]);
			} else
				entry = tracks(gps, &buf[ix]);
			break;
		}

		/* If entry is not null link it into the current list */
		if (entry != NULL) {
			gps_append_list(cur, entry);
			entry = NULL;
		}
	}
	return lists;
}
