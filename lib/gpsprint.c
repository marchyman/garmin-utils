/*
 *	$snafu: gpsprint.c,v 1.23 2003/04/17 23:08:29 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

#include <machine/endian.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gpslib.h"

/*
 * Functions to `print' gps data.  The given packets are formatted
 * and written to stdout.  Formatting varies according to the
 * packet type.
 */

/*
 * GPS time is number of seconds from 12:00 AM Jan 1 1990.  Add this
 * constant to turn it into UNIX time.
 */
#define UNIX_TIME_OFFSET	631065600L

/*
 * Given the address to the start of a `semicircle' in data received
 * from a gps unit convert it to a double.
 */
static double
semicircle2double(const u_char * s)
{
	long work = s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24);
	return work * 180.0 / (double) (0x80000000);
}

#if defined(__vax__)

/*
 * convert the IEEE754 single precision little endian stream to a
 * VAX F floating point data type.
 */
static float
get_float(const u_char * s)
{
	float ret;
	u_int8_t buf[4], p[4];
	int sign, exp, mant;

	/* flip to IEEE754 single precision big endian */
	p[0] = s[3];
	p[1] = s[2];
	p[2] = s[1];
	p[3] = s[0];

	sign = p[0] & 0x80;
	exp = ((p[0] & 0x7f) << 1) | ((p[1] & 0x80) >> 7);
	memset(buf, '\0', sizeof(buf));

	mant = p[1] & 0x7f;
	mant <<= 8;
	mant |= p[2] & 0xff;
	mant <<= 8;
	mant |= p[3] & 0xff;

	if (exp == 0) {
		if (mant == 0) {
			/* true zero */
			goto out;
		} else {
			/* subnormal, fail */
			buf[1] = 0x80;
			goto out;
		}
	}

	if (exp == 255) {
		/* +/- infinity or signaling/quiet NaN, fail */
		buf[1] = 0x80;
		goto out;
	}

	/* Ok, everything else is "normal" */

	exp = exp - 127 + 129;
	buf[0] = ((exp & 1) << 7) | ((mant >> 16) & 0x7f);
	buf[1] = (exp >> 1) | (sign ? 0x80 : 0);
	buf[2] = (mant >> 0) & 0xff;
	buf[3] = (mant >> 8) & 0xff;

out:
	memcpy(&ret, buf, sizeof(ret));
	return (ret);
}

#elif BYTE_ORDER == LITTLE_ENDIAN

/*
 * Magic value used by garmin to signify an "empty" float field.
 */
union {
	u_int32_t u;
	float f;
} no_val = { 0x69045951 };

static float
get_float(const u_char * s)
{
	float f;

	memcpy(&f, s, sizeof(f));
	return (f);
}

#elif BYTE_ORDER == BIG_ENDIAN

/*
 * Magic value used by garmin to signify an "empty" float field.
 */
static union {
	u_int32_t u;
	float f;
} no_val = { 0x51590469 };

static float
get_float(const u_char * s)
{
	float f;
	u_char t[4];

	t[0] = s[3];
	t[1] = s[2];
	t[2] = s[1];
	t[3] = s[0];
	memcpy(&f, t, sizeof(f));
	return (f);
}

#else
# error "unknown float conversion"
#endif

/*
 * Grab a string from a buffer given its offset and length.   Note,
 * the only size requirement is that the offset start before the
 * end of the buffer.
 */
static char *
get_string(const u_char *buf, int bufsiz, u_short off, u_short len)
{
	char *str = NULL;

	if (off != 0 && bufsiz > off) {
		str = malloc(len + 1);
		if (str != NULL)
			strlcpy(str, &buf[off], len + 1);
	}
	return str;
}

/*
 * grab an integer value from a little endian buffer given its offset and
 * length.
 */
static long
get_int(const u_char *buf, int bufsiz, u_short off, u_short len)
{
	int val;

	if (off != 0 && bufsiz >= off + len) {
		val = 0;
		do {
			val <<= 8;
			val += buf[off + --len];
		} while (len);
	} else
		val = -1;
	return val;
}

/*
 * Table of offsets/lengths  for the various fields in each waypoint type
 * needed to print the waypoint.   An offset of 0 indicates that the field
 * does not exist in the given waypoint type.   The packet type is at offset
 * 0, thus the first character of the first field starts at offset 1.
 */
struct wpt_info {
	int	wpt_type;
	u_short	lat_off;	/* length always 4 */
	u_short	long_off;	/* length always 4 */
	u_short	name_off;	/* length always 6 */
	u_short	alt_off;	/* length always 4 */
	u_short	sym_off;
	u_short	sym_len;
	u_short	disp_off;
	u_short	disp_len;
	u_short	cmnt_off;
	u_short	cmnt_len;
};

static struct wpt_info winfo[] = {
	{ D100,  7, 11, 1,  0,  0, 0,  0, 0, 19, 40 },
	{ D101,  7, 11, 1,  0, 63, 1,  0, 0, 19, 40 },
	{ D102,  7, 11, 1,  0, 63, 2,  0, 0, 19, 40 },
	{ D103,  7, 11, 1,  0, 59, 1, 60, 1, 19, 40 },
	{ D104,  7, 11, 1,  0, 63, 2, 65, 1, 19, 40 },
	{ D105,  1,  5, 0,  0,  9, 2,  0, 0, 11, 40 },
	{ D106, 15, 19, 0,  0, 23, 2,  0, 0, 25, 40 },
	{ D107,  7, 11, 1,  0, 59, 1, 60, 1, 19, 40 },
	{ D108, 25, 29, 0, 33,  5, 2,  3, 1, 49, 51 },
	{ D109, 25, 29, 0, 33,  5, 2,  0, 0, 53, 51 }
};

static struct wpt_info *
find_wpt_info(int type)
{
	int ix;
	for (ix = 0; ix < sizeof winfo / sizeof winfo[0]; ix++)
		if (winfo[ix].wpt_type == type)
			return &winfo[ix];
	return NULL;
}

static void
print_waypoint(const u_char *wpt, int len, int type)
{
	struct wpt_info *wi;
	double lat;
	double lon;
	char *name;
	char *cmnt;
	long sym;
	long dsp;
	float alt;

	wi = find_wpt_info(type);
	if (wi != NULL) {
		lat = semicircle2double(&wpt[wi->lat_off]);
		lon = semicircle2double(&wpt[wi->long_off]);
		if (wi->alt_off)
			alt = get_float(&wpt[wi->alt_off]);
		else
			alt = no_val.f;
		name = get_string(wpt, len, wi->name_off, 6);
		sym = get_int(wpt, len, wi->sym_off, wi->sym_len);
		dsp = get_int(wpt, len, wi->disp_off, wi->disp_len);
		cmnt = get_string(wpt, len, wi->cmnt_off, wi->cmnt_len);

		if (alt == no_val.f)
			printf("%s %10f %11f %5ld/%ld %s\n",
			       name == NULL ? "-" : name, lat, lon,
			       sym == -1 ? 0 : sym, dsp == -1 ? 0 : dsp,
			       cmnt == NULL ? "" : cmnt);
		else
			printf("%s %10f %11f %11f %5ld/%ld %s\n",
			       name == NULL ? "-" : name, lat, lon, alt,
			       sym == -1 ? 0 : sym, dsp == -1 ? 0 : dsp,
			       cmnt == NULL ? "" : cmnt);
		if (name)
			free(name);
		if (cmnt)
			free(cmnt);
	} else
		warnx("unknown waypoint packet type: %d", type);
}

/*
 * Table of offsets and lengths for route related packets
 */
struct rte_info {
	int	rte_type;
	int	num_off;
	u_short	cmnt_off;	/* or ident */
	u_short	cmnt_len;
	u_short	class_off;
	u_short class_len;
};

static struct rte_info rinfo[] = {
	{ D200, 1,  0,  0, 0, 0 },
	{ D201, 1,  2, 20, 0, 0 },
	{ D202, 0,  1, 51, 0, 0 },
	{ D210, 0, 21, 51, 1, 2 }
};

static struct rte_info *
find_rte_info(int type)
{
	int ix;
	for (ix = 0; ix < sizeof rinfo / sizeof rinfo[0]; ix++)
		if (rinfo[ix].rte_type == type)
			return &rinfo[ix];
	return NULL;
}

static void
print_route(const u_char *rte, int len, int type)
{
	struct rte_info *ri;
	long num;
	long class;
	char *id;

	ri = find_rte_info(type);
	if (ri != NULL) {
		num = get_int(rte, len, ri->num_off, 1);
		id = get_string(rte, len, ri->cmnt_off, ri->cmnt_len);
		class = get_int(rte, len, ri->class_off, ri->class_len);
		printf("**%ld %s%s\n", num == -1 ? 0 : num, id ? id : "-",
		       class == -1 ? "" :
		       class == 0 ? " line" :
		       class == 1 ? " link" :
		       class == 2 ? " net" :
		       class == 3 ? " direct" :
		       class == 255 ? " snap" : " (unknown class)");
	} else
		warnx("unknown route packet type: %d", type);
}

/*
 * Table of offsets and lengths for track related packets
 */
struct trk_info {
	int	trk_type;
	u_short	lat_off;
	u_short	long_off;
	u_short time_off;
	u_short time_len;
	u_short	alt_off;
	u_short	depth_off;
	u_short	new_off;
	u_short	new_len;
	u_short ident_off;
	u_short ident_len;
};

static struct trk_info tinfo[] = {
	{ D300, 1, 5, 9, 4,  0,  0, 13, 1, 0,  0 },
	{ D301, 1, 5, 9, 4, 13, 17, 21, 1, 0,  0 },
	{ D310, 0, 0, 0, 0,  0,  0,  0, 0, 3, 51 }
};

static struct trk_info *
find_trk_info(int type)
{
	int ix;
	for (ix = 0; ix < sizeof tinfo / sizeof tinfo[0]; ix++)
		if (tinfo[ix].trk_type == type)
			return &tinfo[ix];
	return NULL;
}

/*
 * print a track header or entry.  New entry format:
 *
 *	date       time     lat      long      alt   new
 *	yyyy-mm-dd hh:mm:ss 99.99999 999.99999 99.99 [start]
 */
static void
print_track(const u_char *trk, int len, int type)
{
	struct trk_info *ti;
	char buf[24];
	char *id;
	float lat;
	float lon;
	float alt;
	time_t time;
	long new;

	ti = find_trk_info(type);
	if (ti != NULL) {
		id = get_string(trk, len, ti->ident_off, ti->ident_len);
		if (id) {
			printf("Track: %s\n", id);
			free(id);
		} else {
			lat = semicircle2double(&trk[ti->lat_off]);
			lon = semicircle2double(&trk[ti->long_off]);
			time = get_int(trk, len, ti->time_off, ti->time_len);
			if (time) {
				time += UNIX_TIME_OFFSET;
				strftime(buf, sizeof buf, "%Y-%m-%d %T",
					 gmtime(&time));
			} else
				strlcat(buf, "unknown", sizeof buf);
			if (ti->alt_off)
				alt = get_float(&trk[ti->alt_off]);
			else
				alt = no_val.f;
			/* skip depth for now */
			new = get_int(trk, len, ti->new_off, ti->new_len);
			printf("%s %10f %11f", buf, lat, lon);
			if (alt != no_val.f)
				printf(" %f", alt);
			printf("%s\n", new ? " start" : "");
		}
	} else
		warnx("unknown track packet type: %d", type);
}

static void
print_time(const u_char *utc, int len)
{
	int month = utc [1];
	int day   = utc [2];
	int year  = utc [3] + (utc [4] << 8);
	int hour  = utc [5] + (utc [6] << 8);
	int min   = utc [7];
	int sec   = utc [8];
    
	printf("[UTC %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d]\n", year, month, 
		day, hour, min, sec);
}

int
gps_print(gps_handle gps, enum gps_cmd_id cmd, const u_char *packet,
	  int len) 
{
	static int count;
	static int limit;

	if (packet[0] == p_xfr_end)
		printf("[end transfer, %d/%d records]\n", count, limit);
	else {	
		count += 1;
		switch (packet[0]) {
		case p_xfr_begin:
			count = 0;
			limit = get_int(packet, len, 1, 2);
			printf("%s, %d records]\n",
			       cmd == CMD_RTE ? RTE_HDR :
			       cmd == CMD_TRK ? TRK_HDR :
			       cmd == CMD_WPT ? WPT_HDR : "unknown",
			       limit);
			break;
		case p_wpt_data:
			print_waypoint(packet, len, gps_get_wpt_type(gps));
			break;
		case p_rte_hdr:
			print_route(packet, len, gps_get_rte_hdr_type(gps));
			break;
		case p_rte_wpt_data:
			print_waypoint(packet, len, gps_get_rte_wpt_type(gps));
			break;
		case p_rte_link:
			print_route(packet, len, gps_get_rte_lnk_type(gps));
			break;
		case p_trk_data:
			print_track(packet, len, gps_get_trk_type(gps));
			break;
		case p_utc_data:
			print_time(packet, len);
			break;
		case p_trk_hdr:
			print_track(packet, len, gps_get_trk_hdr_type(gps));
			break;
		default:
			printf("[unknown protocol %d]\n", packet[0]);
		}
	}
	return 0;
}
