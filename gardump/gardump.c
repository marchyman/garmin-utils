/* -*- tab-width: 4; -*-
 * $snafu: gardump.c,v 1.10 2003/04/11 01:21:48 marc Exp $
 *
 * Placed in the Public Domain by Marco S. Hyman
 */

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gpslib.h"

static void
usage(const char* prog, const char* err, ...)
{
	if (err) {
		va_list ap;
		va_start(ap, err);
		vfprintf(stderr, err, ap);
		va_end(ap);
	}
	fprintf(stderr, "usage: %s [-vwrtu] [-d debug-level] [-p port]\n", prog);
	exit(1);
}

int
main(int argc, char * argv[])
{
	int waypoints = 0;
	int routes = 0;
	int tracks = 0;
	int utc = 0;
	int debug = 0;
	const char* port = DEFAULT_PORT;

	int opt;
	char* rem;
	GpsHandle gps;

	while ((opt = getopt(argc, argv, "d:vwrtup:")) != -1) {
		switch (opt) {
		  case 'd':
			debug = strtol(optarg, &rem, 0);
			if (*rem)
				debug = 0;
			if (! debug)
				usage(argv[ 0 ], "`%s' is a bad debug value\n", optarg);
				/* does not return */
			if (debug > 1)
				warnx("debug level set to %d", debug);
			break;
		  case 'v':
			errx(1, "software version %s", VERSION);
			/* does not return */
		  case 'w':
			waypoints = 1;
			if (debug > 1)
				warnx("waypoint dump requested");
			break;
		  case 'r':
			routes = 1;
			if (debug > 1)
				warnx("route dump requested");
			break;
		  case 't':
			if (debug > 1)
				warnx("track dump requested");
			tracks = 1;
			break;
		  case 'u':
			utc = 1;
			if (debug > 1)
				warnx("UTC timestamp requested");
			break;
		  case 'p':
			port = strdup(optarg);
			if (debug > 1)
				warnx("using serial port `%s'", port);
			break;
		  case '?':
		  default:
			usage(argv[ 0 ], 0);
			/* does not return */
		}
	}

	if (argc != optind)
		errx(1, "unknown command line argument: %s ...", argv[ optind ]);
		/* does not return */

	if ((! waypoints) && (! routes) && (! tracks) && (! utc)) {
		waypoints = routes = tracks = utc = 1;
		if (debug > 1)
			warnx("full dump requested");
	}

	gps = gpsOpen(port, debug);
	if (gpsVersion(gps) != 1)
		errx(1, "can't communicate with GPS unit");
		/* does not return */
    if (utc)
		gpsCmd(gps, CMD_UTC);
	if (waypoints)
		gpsCmd(gps, CMD_WPT);
	if (routes)
		gpsCmd(gps, CMD_RTE);
	if (tracks)
		gpsCmd(gps, CMD_TRK);
	gpsClose(gps);

	return 0;
}
