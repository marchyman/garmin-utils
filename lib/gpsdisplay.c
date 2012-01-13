/*
 * Public Domain, 2001, Marco S Hyman <marc@snafu.org>
 */

#include <sys/types.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <strings.h>

#ifdef LINUX
/* Linux puts memset(3) in string.h */
#include <string.h>
#endif


#include "gpslib.h"

/*
 * Debug output function.  Spit out message to stderr only if
 * the current debug output level is >= the given level.
 */
void
gps_printf(gps_handle gps, int level, const char *fmt, ...)
{
	if (gps_debug(gps) >= level) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}

/*
 * Function and variables to dump data to stderr.   Data bytes are printed
 * as decimal values to match the garmin doc.
 */
#define DUMP_BUFLEN		80
#define DUMP_DEC_OFF		4
#define DUMP_ASCII_OFF		(DUMP_DEC_OFF + 44)

/*
 * Write len bytes at buf to stderr.  The `direction' parameter is
 * intended to be '<' to show data received and '>' to show data
 * sent, but any character can be used.
 */
void
gps_display(char direction, const u_char *buf, int len)
{
	u_char	data[DUMP_BUFLEN];

	memset(data, ' ', DUMP_BUFLEN);
	while (len) {
		const u_char *d;
		u_char *h;
		u_char *a;
		int cnt;

		d = buf;
		h = &data[DUMP_DEC_OFF];
		a = &data[DUMP_ASCII_OFF];

		if (len > 10) {
			cnt = 10;
			len -= 10;
		} else {
			cnt = len;
			len = 0;
			memset(data, ' ', DUMP_BUFLEN);
		}
		buf += 10;

		data[1] = (u_char) direction;
		while (cnt--) {
			if (*d < 128 && isprint(*d))
				*a++ = *d;
			else
				*a++ = '.';
			*h++ = (u_char) ('0' + *d / 100);
			*h++ = (u_char) ('0' + *d % 100 / 10);
			*h++ = (u_char) ('0' + *d % 10);
			*h++ = ' ';
			d++;
		}
		*a = 0;
		fprintf(stderr, "%s\n", data);
	}
}

