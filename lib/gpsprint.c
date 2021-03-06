/*
 * Public Domain, 2001, Marco S Hyman <marc@snafu.org>
 */

#include <sys/types.h>

#ifndef LINUX
#include <machine/endian.h>
#else
#include <endian.h>
#endif

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
 * Grab n strings from the given buffer.   Each string is
 * assumed to be NULL terminated with a maximum of 50 characters
 * before the NULL.   Enforce this by truncation if necessary.
 */
#define STRING_MAX 51
static void
get_strings(const u_char *buf, int bufsiz, u_short off, char *strings[],
	    int string_cnt)
{
	int ix;

	for (ix = 0; ix < string_cnt; ix++) {
		strings[ix] = malloc(STRING_MAX);
		if (strings[ix] != NULL) {
			if (off > 0 && bufsiz > off)
				off += strlcpy(strings[ix],
					       (const char *) &buf[off],
					       STRING_MAX) + 1;
			else
				*strings[ix] = 0;
		}
	}
}

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
		str = malloc((size_t) len + 1);
		if (str != NULL)
			strlcpy(str, (const char *) &buf[off],
				(size_t) len + 1);
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
 * does not exist in the given waypoint type.   A length of zero indicates
 * the beginning of a table of null terminated strings.
 * The packet type is at offset 0, thus the first character of the first
 * field starts at offset 1.
 */
struct wpt_info {
	int	wpt_type;
	u_short	lat_off;	/* length always 4 */
	u_short	long_off;	/* length always 4 */
	u_short	name_off;	/* length always 6 */
	u_short	alt_off;	/* length always 4 */
	u_short	sym_off;
	u_short	sym_len;
	u_short	disp_off;	/* length always 1 */
	u_short	class_off;	/* length always 1 */
	u_short	subclass_off;
	u_short	subclass_len;
	u_short	cmnt_off;
	u_short	cmnt_len;
};

static struct wpt_info winfo[] = {
/*	  typ  lat lon alt nam -sym- dsp cls -sub-  -cmnt- */
	{ D100,  7, 11, 1,  0,  0, 0,  0, 0, 0, 0,  19, 40 },
	{ D101,  7, 11, 1,  0, 63, 1,  0, 0, 0, 0,  19, 40 },
	{ D102,  7, 11, 1,  0, 63, 2,  0, 0, 0, 0,  19, 40 },
	{ D103,  7, 11, 1,  0, 59, 1, 60, 0, 0, 0,  19, 40 },
	{ D104,  7, 11, 1,  0, 63, 2, 65, 0, 0, 0,  19, 40 },
	{ D105,  1,  5, 0,  0,  9, 2,  0, 0, 0, 0,  11,  0 },
	{ D106, 15, 19, 0,  0, 23, 2,  0, 1, 2, 13, 25,  0 },
	{ D107,  7, 11, 1,  0, 59, 1, 60, 0, 0, 0,  19, 40 },
	{ D108, 25, 29, 0, 33,  5, 2,  3, 1, 7, 18, 49,  0 },
	{ D109, 25, 29, 0, 33,  5, 2,  0, 2, 7, 18, 53,  0 }
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
	const u_char *w;
	double lat;
	double lon;
	char *name;
	char *cmnt[2];
	long sym;
	long dsp;
	long class;
	float alt;

	wi = find_wpt_info(type);
	if (wi != NULL) {
		lat = gps_semicircle2double(&wpt[wi->lat_off]);
		lon = gps_semicircle2double(&wpt[wi->long_off]);
		printf("%12.8f %13.8f", lat, lon);

		if (wi->alt_off)
			alt = gps_get_float(&wpt[wi->alt_off]);
		else
			alt = no_val.f;
		if ((alt != no_val.f) && alt < 5.0e24)
			printf(" A:%11f",alt);

		sym = get_int(wpt, len, wi->sym_off, wi->sym_len);
		if (sym != -1)
			printf(" S:%ld", sym);

		dsp = get_int(wpt, len, wi->disp_off, 1);
		if (dsp != -1) {
			printf(" D:%ld", dsp);
		}

		if (wi->name_off) {
			/* old style: fixed len name/comment */
			name = get_string(wpt, len, wi->name_off, 6);
			if (name) {
				printf(" I:%s", name);
				free(name);
			}
			cmnt[0] = get_string(wpt, len, wi->cmnt_off,
					     wi->cmnt_len);
			if (cmnt[0]) {
				printf(" C:%s", cmnt[0]);
				free(cmnt[0]);
			}
		} else {
			/* new style: null terminated ident/comment */
			get_strings(wpt, len, wi->cmnt_off, cmnt, 2);
			if (cmnt[0]) {
				if (*cmnt[0])
					printf(" I:%s", cmnt[0]);
				free(cmnt[0]);
			}
			if (cmnt[1]) {
				if (*cmnt[1])
					printf(" C:%s", cmnt[1]);
				free(cmnt[1]);
			}
		}

		/*
		 * Newer GPS contain a class/subclass that describes
		 * map points.   Save that information as a hex string
		 * if it exists and is not zero (zero is a user waypoint).
		 */

		class = get_int(wpt, len, wi->class_off, 1);
		if (class != -1 && class != 0) {
			printf(" W:%02x", (int) class);
			for (w = &wpt[wi->subclass_off];
			     w < &wpt[wi->subclass_off + wi->subclass_len]; w++)
				printf("%02x", *w);
		}
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
	{ D202, 0,  1,  0, 0, 0 },
	{ D210, 0, 21,  0, 1, 2 }
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
	char *id;

	ri = find_rte_info(type);
	if (ri != NULL) {
		num = get_int(rte, len, ri->num_off, 1);
		if (ri->cmnt_len)
			id = get_string(rte, len, ri->cmnt_off, ri->cmnt_len);
		else
			get_strings(rte, len, ri->cmnt_off, &id, 1);
		printf("**%ld %s\n", num == -1 ? 0 : num, id ? id : "");
	} else
		warnx("unknown route packet type: %d", type);
}

static void
print_route_link(const u_char *rte, int len, int type)
{
	struct rte_info *ri;
	long class;

	ri = find_rte_info(type);
	if (ri != NULL) {
		class = get_int(rte, len, ri->class_off, ri->class_len);
		if (class != -1)
			printf(" L:%ld\n", class);
	} else
		warnx("unknown route link type: %d", type);
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
 *	date       time       lat      long      alt   new
 *	[yyyy-mm-dd hh:mm:ss] 99.99999 999.99999 99.99 [start]
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
	time_t tim;
	long new;

	ti = find_trk_info(type);
	if (ti != NULL) {
		id = get_string(trk, len, ti->ident_off, ti->ident_len);
		if (id) {
			printf("Track: %s\n", id);
			free(id);
		} else {
			lat = (float) gps_semicircle2double(&trk[ti->lat_off]);
			lon = (float) gps_semicircle2double(&trk[ti->long_off]);
			tim = (time_t) get_int(trk, len, ti->time_off,
					       ti->time_len);
			if (tim != -1) {
				tim += UNIX_TIME_OFFSET;
				strftime(buf, sizeof buf, "%Y-%m-%d %T ",
					 gmtime(&tim));
			} else
				buf[0] = 0;
			if (ti->alt_off)
				alt = gps_get_float(&trk[ti->alt_off]);
			else
				alt = no_val.f;
			/* skip depth for now */
			new = get_int(trk, len, ti->new_off, ti->new_len);
			printf("%s%12.8f %13.8f", buf, lat, lon);
			if (alt != no_val.f)
				printf(" %f", alt);
			printf("%s\n", new ? " start" : "");
		}
	} else
		warnx("unknown track packet type: %d", type);
}

static void
print_time(const u_char *utc)
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

/*
 * print a screenshot in PPM format (use with redirect of stdout to a file)
 *
 * also trys to retrieve pressure reading from top left field of Altimeter
 * display and print it to stderr (only works if Altimeter display is
 * displayed in daytime mode and with pressure units set to inHg)
 *
 * (tested only on Garmin GPSmap 76CS)
 *
 * Wolfgang Baudler <wbaudler@gb.nrao.edu>
 *
 */
static void
print_screenshot(const u_char *packet, int len)
{
	static int r[256], g[256], b[256];
	static int j = 0;
	static int x = 0, y = 0;
	static unsigned char byte = 0;
	static int bitcount = 0;
	int i;
	int k;
	unsigned int digits_id[] = {
		0x438, 0x249, 0x26B, 0x267, 0x3C6,
		0x26F, 0x35F, 0x2A3, 0x4C7, 0x44F
	};

	static unsigned int digit[4] = { 0, 0, 0, 0 };
	int xlow[4] = { 8, 22, 40, 54 };
	int xhigh[4] = { 20, 34, 52, 66 };
  
	static int printed = 0;

	/* retrieve image size and output to PPM header  */
	if ( j == 0 )
		printf("P6\n%d,%d\n255\n", packet[17], packet[21]); 
	else if (j >=1 && j <256) {
		/* retrieve and store palette info */  
		b[j-1] = packet[9];
		g[j-1] = packet[10];
		r[j-1] = packet[11];
	} else if (j >= 257) {
		/* image data */
		/* process packet less header */
		for (i=9; i<len; i++) { 
			/* write RGB pixels */
			fputc(r[packet[i]],stdout);
			fputc(g[packet[i]],stdout);
			fputc(b[packet[i]],stdout);

			/* determine digits for pressure reading */
	
			for (k=0; k<4; k++) { 
				if ((x >= xlow[k] && x <= xhigh[k]) &&
				    (y >= 44 && y <=46 )) { 
					byte = byte |
					  ((r[packet[i]] != 255)<<(7-bitcount));
					bitcount++;
					if ((bitcount == 8) ||
					    (x == xhigh[k])) {
						digit[k] += byte;
						bitcount = 0;
						byte=0;
					}
				}
			}

			/* update x and y coordinate counters of image */
			x++;
			if ( x == 160) {
				x=0;
				y++;
			}
		}

		/* convert and print pressure value */
		if (y > 47 && !printed) { 
			for (k = 0; k < 4; k++) {
				i=0;
				while ((digits_id[i] != digit[k]) && (i<=9))
					i++;
				digit[k]=i;
			}

			printed = 1;

			fprintf(stderr,
				"[Altimeter Screen, "
				"top left field: %d%d.%d%d inHg]\n",
				digit[0], digit[1], digit[2], digit[3] );
		} 
	}  
	j++;
}

int
gps_print(gps_handle gps, enum gps_cmd_id cmd, const u_char *packet,
	  int len) 
{
	static int count;
	static int limit;
	static int rte_newline;

	if (packet[0] == p_xfr_end) {
		if (rte_newline) {
			rte_newline = 0;
			printf("\n");
		}
		printf("[end transfer, %d/%d records]\n", count, limit);
	} else {	
		count += 1;
		switch (packet[0]) {
		case p_xfr_begin:
			rte_newline = 0;
			count = 0;
			limit = (int) get_int(packet, len, 1, 2);
			switch (cmd) {
			case CMD_RTE:
				printf(RTE_HDR ", %d records]\n"
				       "# **n [route name]\n"
				       "# lat long [A:alt] [S:sym] "
				       "[D:display] [I:id] [C:cmnt] "
				       "[W:wpt info] [L:link]\n", limit);
				break;
			case CMD_TRK:
				printf(TRK_HDR ", %d records]\n"
				       "# [Track: track name]\n"
				       "# [yyyy-mm-dd hh:mm:ss] lat long [alt] "
				       "[start]\n", limit);
				break;
			case CMD_WPT:
				printf(WPT_HDR ", %d records]\n"
				       "# **n [route name]\n"
				       "# lat long [A:alt] [S:sym] "
				       "[D:display] [I:id] [C:cmnt] "
				       "[W:wpt info] [L:link]\n", limit);
				break;
			default:
				printf("[unknown, %d records]\n", limit);
				break;
			}
			break;
		case p_wpt_data:
			print_waypoint(packet, len, gps_get_wpt_type(gps));
			printf("\n");
			break;
		case p_rte_hdr:
			if (rte_newline) {
				rte_newline = 0;
				printf("\n");
			}
			print_route(packet, len, gps_get_rte_hdr_type(gps));
			break;
		case p_rte_wpt_data:
			if (rte_newline) {
				rte_newline = 0;
				printf("\n");
			}
			print_waypoint(packet, len, gps_get_rte_wpt_type(gps));
			rte_newline = 1;
			break;
		case p_rte_link:
			print_route_link(packet, len,
					 gps_get_rte_lnk_type(gps));
			rte_newline = 0;
			break;
		case p_trk_data:
			print_track(packet, len, gps_get_trk_type(gps));
			break;
		case p_utc_data:
			print_time(packet);
			break;
		case p_trk_hdr:
			print_track(packet, len, gps_get_trk_hdr_type(gps));
			break;
		case p_scr_shot:
			print_screenshot(packet, len);
			break;
		default:
			printf("[unknown protocol %d]\n", packet[0]);
		}
	}
	return 0;
}
