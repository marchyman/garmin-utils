/* -*- tab-width: 4; -*-
 * $snafu: garload.c,v 1.9 2002/09/10 00:18:43 marc Exp $
 *
 * Placed in the Public Domain by 1998 Marco S. Hyman
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <err.h>

#include "gps1.h"
#include "gpsversion.h"
#include "gpsformat.h"
#include "gpsload.h"

static void
usage(const char* prog, const char* err, ...)
{
	if (err) {
		va_list ap;
		va_start(ap, err);
		vfprintf(stderr, err, ap);
		va_end(ap);
	}
	fprintf(stderr, "usage: %s [-v] [-d debug-level] [-p port]\n", prog);
	exit(1);
}

int
main(int argc, char * argv[])
{
	int debug = 0;
	const char* port = DEFAULT_PORT;

	int opt;
	char* rem;
	GpsHandle gps;
	GpsLists * lists;

	while ((opt = getopt(argc, argv, "d:vp:")) != -1) {
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
		  case 'p':
			port = strdup(optarg);
			if (debug > 1)
				warnx("using serial port `%s'", port);
			break;
		  case 'v':
			errx(1, "software version %s", VERSION);
			/* does not return */
		  case '?':
		  default:
			usage(argv[ 0 ], 0);
			/* does not return */
		}
	}

	if (argc != optind)
		errx(1, "unknown command line argument: %s ...", argv[ optind ]);
		/* does not return */

	gps = gpsOpen(port, debug);
    if (gpsVersion(gps) != 1)
		errx(1, "can't communicate with GPS unit");
		/* does not return */
    lists = gpsFormat(gps, stdin);
    if (! lists)
		errx(1, "no valid GPS data found");
		/* does not return */
    if (gpsLoad(gps, lists) < 0)
		errx(1, "failure uploading GPS unit");
    return 0;
}
