/* -*- tab-width: 4; -*-
 * $snafu: gardump.c,v 1.12 2003/04/13 18:01:16 marc Exp $
 *
 * Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

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
		fprintf(stderr, "usage: %s [-vwrtu] [-d debug-level] [-p port]\n",
				prog);
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
		gps_handle gps;

		while ((opt = getopt(argc, argv, "d:vwrtup:")) != -1) {
				switch (opt) {
				case 'd':
						debug = strtol(optarg, &rem, 0);
						if (*rem)
								debug = 0;
						if (! debug)
								usage(argv[ 0 ], "`%s' is a bad debug value\n",
									  optarg);
						break;
				case 'v':
						errx(1, "software version %s", VERSION);
						/* does not return */
				case 'w':
						waypoints = 1;
						break;
				case 'r':
						routes = 1;
						break;
				case 't':
						tracks = 1;
						break;
				case 'u':
						utc = 1;
						break;
				case 'p':
						port = strdup(optarg);
						break;
				case '?':
				default:
						usage(argv[ 0 ], 0);
						/* does not return */
				}
		}

		if (argc != optind)
				errx(1, "unknown command line argument: %s ...", argv[optind]);
				/* does not return */

		if (!waypoints && !routes && !tracks && !utc)
				waypoints = routes = tracks = utc = 1;

		gps = gps_open(port, debug);
		if (gps_version(gps) != 1)
				errx(1, "can't communicate with GPS unit");
				/* does not return */
		printf("[gardump version %s]\n", VERSION);
		if (utc)
				gps_cmd(gps, CMD_UTC);
		if (waypoints)
				gps_cmd(gps, CMD_WPT);
		if (routes)
				gps_cmd(gps, CMD_RTE);
		if (tracks)
				gps_cmd(gps, CMD_TRK);
		gps_close(gps);

		return 0;
}
