/* -*- tab-width: 4; -*-
 * $snafu: garload.c,v 1.11 2003/04/11 21:17:15 marc Exp $
 *
 * Placed in the Public Domain by 1998 Marco S. Hyman
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
		gps_handle gps;
		struct gps_lists *lists;

		while ((opt = getopt(argc, argv, "d:vp:")) != -1) {
				switch (opt) {
				case 'd':
						debug = strtol(optarg, &rem, 0);
						if (*rem)
								debug = 0;
						if (!debug)
								usage(argv[0], "`%s' is a bad debug value\n",
									  optarg);
						break;
				case 'p':
						port = strdup(optarg);
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
				errx(1, "unknown command line argument: %s ...", argv[optind]);
				/* does not return */

		gps = gps_open(port, debug);
		if (gps_version(gps) != 1)
				errx(1, "can't communicate with GPS unit");
				/* does not return */
		lists = gps_format(gps, stdin);
		if (!lists)
				errx(1, "no valid GPS data found");
				/* does not return */
		if (gps_load(gps, lists) < 0)
				errx(1, "failure uploading GPS unit");
		return 0;
}
