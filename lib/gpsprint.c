/*
 *	$snafu: gpsprint.c,v 1.7 2001/06/13 22:21:27 marc Exp $
 *
 *	Copyright (c) 1998 Marco S. Hyman
 *
 *	Permission to copy all or part of this material with or without
 *	modification for any purpose is granted provided that the above
 *	copyright notice and this paragraph are duplicated in all copies.
 *
 *	THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sys/types.h>

#include <stdio.h>
#include <time.h>

#include "gpsproto.h"
#include "gps1.h"
#include "gpsdump.h"
#include "gpsprint.h"

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
semicircleToDouble( const unsigned char * s )
{
    long work = s[ 0 ] + ( s[ 1 ] << 8 ) + ( s[ 2 ] << 16 ) + ( s[ 3 ] << 24 );
    return work * 180.0 / (double) (0x80000000);
}


    /*
     * Waypoint format is:
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
     * printed as:
     * xxxxxx -99.999999 -999.999999 sssss/d comments
     *
     *
     * GPS 12/12XL waypoint format is:
     *
     * ...
     *	59	symbol
     *	60	display option (0: sym + name, 1: symbol, 2: sym + comment)
     *
     * printed as:
     * xxxxxx -99.999999 -999.999999 ss/d comments
     *
     */
static void
printWaypoint( const unsigned char * wpt, int len )
{
    unsigned char name[ 8 ];
    unsigned char comment[ 44 ];
    double lat = semicircleToDouble( &wpt[ 7 ] );
    double lon = semicircleToDouble( &wpt[ 11 ] );
    int sym = 0;
    int disp = 0;

    memcpy( name, &wpt[ 1 ], 6 );
    name[ 6 ] = 0;
    memcpy( comment, &wpt[ 19 ], 40 );
    comment[ 40 ] = 0;

#if GPS == 0   /* GPS III */
    if ( len >= 65 ) {
	sym = wpt[ 63 ] + ( wpt[ 64 ] << 8 );
	if ( len >= 66 ) {
	    disp = wpt[ 65 ];
	}
    }
    printf( "%s %10f %11f %5d/%d %s\n", name, lat, lon,
	    sym, disp, comment );
#elif GPS == 1  /* GPS 12/12XL */
    if ( len >= 60 ) {
	sym = wpt[ 59 ];
	if ( len >= 61 ) {
	    disp = wpt[ 60 ];
	}
    }
    printf( "%s %10f %11f %2d/%1d %s\n", name, lat, lon,
	    sym, disp, comment );
#else
#error Unknown GPS value
#endif
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
printRteHdr( const unsigned char * hdr, int len )
{
    unsigned char comment[ 24 ];

    if ( len > 2 ) {
	memcpy( comment, &hdr[ 2 ], 20 );
	comment[ 20 ] = 0;
    } else {
	comment[ 0 ] = 0;
    }
    printf( "**%d %s\n", hdr[ 1 ], comment );
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
printTrack( const unsigned char * trk, int len )
{
    double lat = semicircleToDouble( &trk[ 1 ] );
    double lon = semicircleToDouble( &trk[ 5 ] );
    time_t time = UNIX_TIME_OFFSET + trk[ 9 ] + ( trk[ 10 ] << 8 ) +
		  ( trk[ 11 ] << 16 ) + ( trk[ 12 ] << 24 );
    const char * startstring = trk[ 13 ] ? " start" : "";
    char timestring[ 32 ];

    if ( time > 0 ) {
	struct tm * gmt = gmtime( &time );
	snprintf( timestring, 32, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
		 gmt->tm_year + 1900, gmt->tm_mon + 1, gmt->tm_mday,
		 gmt->tm_hour, gmt->tm_min, gmt->tm_sec );
    } else {
	strcpy( timestring, "unknown" );
    }
    printf( "%10f %11f %s%s\n", lat, lon, timestring, startstring );
}

int
gpsPrint( GpsHandle gps, GpsCmdId cmd, const unsigned char * packet, int len )
{
    const char * type;
    static int count;
    static int limit;

    if ( packet[ 0 ] == xfrEnd ) {
	printf( "[end transfer, %d/%d records]\n", count, limit );
    } else {	
	count += 1;
	switch ( packet[ 0 ] ) {
	  case xfrBegin:
	    count = 0;
	    limit = packet[ 1 ] + ( packet[ 2 ] << 8 );
	    switch ( cmd ) {
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
	    printf( "[%s, %d records]\n", type, limit );
	    break;
	  case rteHdr:
	    printRteHdr( packet, len );
	    break;
	  case rteWptData:
	    printWaypoint( packet, len );
	    break;
	  case trkData:
	    printTrack( packet, len );
	    break;
	  case wptData:
	    printWaypoint( packet, len );
	    break;
	  default:
	    printf( "[unknown protocol %d]\n", packet[ 0 ] );
	}
    }
    return 0;
}
