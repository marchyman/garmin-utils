/*
 *	$snafu: gpsprint.c,v 1.14 2003/04/11 01:21:49 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

#include <machine/endian.h>

#include <err.h>
#include <stdio.h>
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
semicircle2double(const unsigned char * s)
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
get_float(const unsigned char * s)
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

static float
get_float(const unsigned char * s)
{
	float f;

	memcpy(&f, s, sizeof(f));
	return (f);
}

#elif BYTE_ORDER == BIG_ENDIAN

static float
get_float(const unsigned char * s)
{
	float f;
	unsigned char t[4];

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
 * Table of offsets/lengths  for the various fields in each waypoint type
 * needed to print the waypoint.   An offset of 0 indicates that the field
 * does not exist in the given waypoint type.   A length of 0 indicates
 * the field is null terminated.   The packet type is at offset 0, so the
 * first character of the first field starts at offset 1.
 */
struct wpt_info {
	int	wpt_type;
	u_short	lat_off;
	u_short	long_off;
	u_short	name_off;
	u_short	name_len;
	u_short	sym_off;
	u_short	sym_len;
	u_short	disp_off;
	u_short	disp_len;
	u_short	comment_off;
	u_short	comment_len;
};

static struct wpt_info winfo[] = {
	{ D100,  7, 11,  1, 6,  0, 0,  0, 0, 19, 40 },
	{ D101,  7, 11,  1, 6, 63, 1,  0, 0, 19, 40 },
	{ D102,  7, 11,  1, 6, 63, 2,  0, 0, 19, 40 },
	{ D103,  7, 11,  1, 6, 59, 1, 60, 1, 19, 40 },
	{ D104,  7, 11,  1, 6, 63, 2, 65, 1, 19, 40 },
	{ D105,  1,  5,  0, 0,  9, 2,  0, 0, 11,  0 },
	{ D106, 15, 19,  0, 0, 23, 2,  0, 0, 25,  0 },
	{ D107,  7, 11, 59, 60, 19 },
	{ D108, 49, 25, 29, 5, -1, -1 },

/*
 * D100 waypoint format is:
 *   GPS 38/40/45 and GPS II
 *   xxxxxx -99.999999 -999.999999 0/0 comments
 *
 *	 1	packet type
 *	 6	name
 *	 4	lat
 *	 4	long
 *	 4	unused
 *	40	comment
 *
 *
 * D103 waypoint format is:
 *   GPS 12/12XL/48 and GPS II+
 *
 * ...
 *	59	symbol
 *	60	display option (0: sym + name, 1: symbol, 2: sym + comment)
 *
 *
 * D104 waypoint format is:
 *   GPS III/III+
 *   xxxxxx -99.999999 -999.999999 sssss/d comments
 *
 *	 1	packet type
 *	 6	name
 *	 4	lat
 *	 4	long
 *	 4	unused
 *	40	comment
 *	 4	distance in meters (float)
 *	 2	symbol
 *	 1	display option (1: symbol, 3: sym + name, 5: sym + comment
 *
 *
 * D108 waypoint format is:
 *  GPSMAP eMap
 *
 *	 1	packet type
 *	 1	class
 *	 1	color
 *	 1	display options
 *	 1	attributes
 *	 2	symbol type
 *	18	sub-class
 *	 4	lat
 *	 4	long
 *	 4	alt
 *	 4	depth
 *	 4	distance
 *	 2	state
 *	 2	country
 *	 var	ident, comment, facility, city, addr, cross_road
 *
 * D109 waypoint format is
 */
static void
printWaypoint(const unsigned char * wpt, int len, int wptType)
{
    unsigned char name[8];
    unsigned char comment[44];
    double lat = semicircle2double(&wpt[7]);
    double lon = semicircle2double(&wpt[11]);
    int sym = 0;
    int disp = 0;

    memcpy(name, &wpt[1], 6);
    name[6] = 0;
    memcpy(comment, &wpt[19], 40);
    comment[40] = 0;
 
    if (wptType == D100) {
        printf("%s %10f %11f 0/0 %s\n", name, lat, lon, comment);
    } else if (wptType == D103) {
        if (len >= 60) {
           sym = wpt[59];
           if (len >= 61) {
               disp = wpt[60];
           }
        }
        printf("%s %10f %11f %2d/%1d %s\n", name, lat, lon,
	        sym, disp, comment);
    } else if (wptType == D104) {                                                                
        memcpy(name, &wpt[1], 6);
        name[6] = 0;
        memcpy(comment, &wpt[19], 40);
        comment[40] = 0;

        if (len >= 65) {
            sym = wpt[63] + (wpt[64] << 8);
            if (len >= 66) {
                disp = wpt[65];
            }
        }
        printf("%s %10f %11f %5d/%d %s\n", name, lat, lon,
	        sym, disp, comment);
	/* incomplete code from Stefan Cermak <cermak@emt.tugraz.at>
	   for the etrex */
    } else if (wptType == D108) {
 	unsigned char offset_comment;
 	double lat = semicircle2double(&wpt[25]);
 	double lon = semicircle2double(&wpt[29]);
 	sym= wpt[5];
 	disp= wpt[3];
 	offset_comment=49+strlen(&wpt[49])+1;
 	printf("%s %10f %11f %d/%d %s\n", &wpt[49], lat, lon, sym,
  		    disp, &wpt[offset_comment]);
    } else {
        warnx ("unknown waypoint packet type: %d", wptType);
    }
}

    /*
     * print a route header.  The format is:
     *	 1	packet type
     *	 1	route number
     *	20	comment
     *
     * printed as:
     * route=999 optional comments
     */
static void
printRteHdr(const unsigned char * hdr, int len, int type)
{
    unsigned char comment[24];

    switch (type) {
    case D200:
	printf("**%d\n", hdr[1]);
	break;
    case D201:
	memcpy(comment, &hdr[2], 20);
	comment[20] = 0;
	printf("**%d %s\n", hdr[1], comment);
	break;
    case D202:
	printf("**%s\n", &hdr[1]);
	break;
    default:
	printf("unknown rte hdr: D%03d\n", type);
    }
}

static void
printRteLink(const unsigned char *lnk, int len)
{
	const char *linktype = NULL;

	switch (lnk[1] | (lnk[2] << 8)) {
	case 0:
		linktype = "line";
		break;
	case 1:
		linktype = "link";
		break;
	case 2:
		linktype = "net";
		break;
	case 3:
		linktype = "direct";
		break;
	case 0xff:
		linktype = "snap";
		break;
	default:
		linktype = NULL;
		break;
	}

	printf(" link");
	if (linktype == NULL)
		printf(" 0x%x",  lnk[1] | (lnk[2] << 8));
	else
		printf(" %s", linktype);

	if (lnk[21] != '\0')
		printf(" %s", &lnk[21]);
	printf("\n");
}

    /*
     * print a track.  The format is:
     *	 1	packet type
     *	 4	lat
     *	 4	long
     *	 4	time
     *	 1	new track flag
     *
     * printed as:
     * -99.999999 -999.999999 yyyy-mm-dd hh:mm:ss [start]
     *
     */
static void
printTrack(const unsigned char * trk, int len, int type)
{
    double lat, lon, alt, dpth;
    time_t time;
    const char * startstring;
    char timestring[32];

    switch (type) {
    case D300:
	lat = semicircle2double(&trk[1]);
	lon = semicircle2double(&trk[5]);
	time = UNIX_TIME_OFFSET + trk[9] + (trk[10] << 8) +
		  (trk[11] << 16) + (trk[12] << 24);
	startstring = trk[13] ? " start" : "";
	printf("%10f %11f %s%s\n", lat, lon, timestring, startstring);
	break;
    case D301:
	lat = semicircle2double(&trk[1]);
	lon = semicircle2double(&trk[5]);
	time = UNIX_TIME_OFFSET + trk[9] + (trk[10] << 8) +
		  (trk[11] << 16) + (trk[12] << 24);
	alt = get_float(&trk[13]);
	dpth = get_float(&trk[17]);
	startstring = trk[21] ? " start" : "";
	if (time > 0) {
	    struct tm * gmt = gmtime(&time);
	    snprintf(timestring, 32, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
		 gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
		 gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	} else {
	    strcpy(timestring, "unknown");
	}
	printf("%10f %11f %f %f %s%s\n", lat, lon, alt, dpth,
	    timestring, startstring);
	break;
    default:
	printf("[unknown trk point type %d\n", type);
    }
}

static void
printTime(const unsigned char * utc, int len)
{
    int month = utc [1];
    int day   = utc [2];
    int year  = utc [3] + (utc [4] << 8);
    int hour  = utc [5] + (utc [6] << 8);
    int min   = utc [7];
    int sec   = utc [8];
    
    printf ("[UTC %4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d]\n", year, month, 
            day, hour, min, sec);
}

static void
printTrackHdr(const unsigned char * trk, int len)
{
	printf("Track: %s\n", trk + 3);
}

int
gpsPrint(GpsHandle gps, GpsCmdId cmd, const unsigned char * packet, int len)
{
    const char * type;
    static int count;
    static int limit;

    if (packet[0] == xfrEnd) {
	printf("[end transfer, %d/%d records]\n", count, limit);
    } else {	
	count += 1;
	switch (packet[0]) {
	  case xfrBegin:
	    count = 0;
	    limit = packet[1] + (packet[2] << 8);
	    switch (cmd) {
	      case CMD_RTE:
		type = "routes";
		break;
	      case CMD_TRK:
		type = "tracks";
		break;
	      case CMD_WPT:
		type = "waypoints";
		break;
	      default:
		type = "unknown";
		break;
	    }	    
	    printf("[%s, %d records]\n", type, limit);
	    break;
	  case rteHdr:
	    printRteHdr(packet, len, gpsGetRteRteHdr(gps));
	    break;
	  case rteWptData:
	    printWaypoint(packet, len, gpsGetWptType(gps));
	    break;
	  case rteLink:
	    printRteLink(packet, len);
	    break;
	  case trkData:
	    printTrack(packet, len, gpsGetTrkTrkType(gps));
	    break;
	  case wptData:
	    printWaypoint(packet, len, gpsGetWptType(gps));
	    break;
	  case utcData:
	    printTime(packet, len);
	    break;
	  case trkHdr:
	    printTrackHdr(packet, len);
	    break;
	  default:
	    printf("[unknown protocol %d]\n", packet[0]);
	}
    }
    return 0;
}
