/* 
 * $snafu: gpsfloat.c,v 2.1 2004/08/19 02:45:29 marc Exp $
 *
 * PUBLIC DOMAIN: No Rights Reserved.  Marco S Hyman <marc@snafu.org>
 */

/*
 * Handle floating point conversion for various endian architectures.
 */

#include <sys/types.h>

#ifndef LINUX
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gpslib.h"

/*
 * Given the address to the start of a `semicircle' in data received
 * from a gps unit convert it to a double.
 */
double
gps_semicircle2double(const u_char * s)
{
	long work = s[0] + (s[1] << 8) + (s[2] << 16) + (s[3] << 24);
	return work * 180.0 / (double) (0x80000000);
}

#if defined(__vax__)

/*
 * convert the IEEE754 single precision little endian stream to a
 * VAX F floating point data type.
 */
float
gps_get_float(const u_char * s)
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
	return ret;
}

#elif BYTE_ORDER == LITTLE_ENDIAN

/*
 * Magic value used by garmin to signify an "empty" float field.
 */
union no_val no_val = { 0x69045951 };

float
gps_get_float(const u_char * s)
{
	float f;

	memcpy(&f, s, sizeof(f));
	return f;
}

int
gps_put_float(u_char *s, float f)
{
	memcpy(s, &f, sizeof(f));
	return sizeof f;
}

#elif BYTE_ORDER == BIG_ENDIAN

/*
 * Magic value used by garmin to signify an "empty" float field.
 */
union no_val no_val = { 0x51590469 };

float
gps_get_float(const u_char * s)
{
	float f;
	u_char t[sizeof f];

	t[0] = s[3];
	t[1] = s[2];
	t[2] = s[1];
	t[3] = s[0];
	memcpy(&f, t, sizeof f);
	return f;
}

int
gps_put_float(u_char *s, float f)
{
	u_char t[sizeof f];
	
	memcpy(t, &f, sizeof(f));
	s[0] = t[3];
	s[1] = t[2];
	s[2] = t[1];
	s[3] = t[0];
	return sizeof f;
}

#else
# error "unknown float conversion"
#endif

