/*
 *	$snafu: gpsversion.c,v 1.8 2003/04/13 18:01:17 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gpslib.h"

/*
 * Request the unit version and output to stdout.  Wait for a
 * protocol capabilities packet and consume it if seen.
 *
 *	-1:	command failed
 *	1:	command succeeded.
 */
int
gps_version(gps_handle gps)
{
	int product_id;
	int software_version;
	char *product_description;

	if (gps_product(gps, &product_id, &software_version,
			&product_description)) {
		gps_printf(gps, 1, __func__ ": failed\n");
		return -1;
	}
	printf("[product %d, version %d: %s]\n", product_id, software_version,
	       product_description ? product_description : "unknown");
	if (product_description)
		free(product_description);

	/* Grab the protocol capabilities packet if it is there so it doesn't
	   screw up anything else.  We don't use it.  Some units send it
	   every time the product description is requested. */

	gps_protocol_cap(gps);
	return 1;
}
