/*
 *	$snafu: gpsprod.c,v 1.8 2003/04/14 07:16:21 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gpslib.h"

/*
 * Garmin GPS product data application protocol
 *
 * host -> gps:	product request
 * gps -> host:	product data
 */

/*
 * Retrieve the product information from the unit specified by
 * `gps' and return product information in productId, softwareVersion,
 * and productDescription.  *productDescription is allocated via
 * malloc and should be free'd when no longer needed.
 *
 * A product request is protocol ID 254, its response is 255.
 *
 * procedure returns -1 on error, otherwise 0.
 */
int
gps_product(gps_handle gps, int *product_id, int *software_version,
	    char **product_description)
{
	char rqst = p_prod_rqst;
	int retries = 5;
	u_char *data = malloc(GPS_FRAME_MAX);

	if (! data) {
		gps_printf(gps, 0, __func__ ": no memory\n");
		return -1;
	}

	gps_printf(gps, 3, __func__ ": send\n");

	while (retries--) {
		if (gps_send_wait(gps, &rqst, 1, 5) == 1) {
			int datalen = GPS_FRAME_MAX;
			if (gps_recv(gps, 5, data, &datalen) == 1) {
				if (data[0] == p_prod_resp) {
					gps_send_ack(gps, *data);
					*product_id = data[1] + (data[2] << 8);
					*software_version =
						data[3] + (data[4] << 8);
					if (datalen > 5)
						*product_description =
							strdup(&data[5]);
					else
						*product_description = 0;
					gps_printf(gps, 3, __func__
						   ": rcvd\n");
					free(data);
					return 0;
				}
			}
			gps_send_nak(gps, *data);
			gps_printf(gps, 3, __func__ ": retry\n");
		}
	}
	gps_printf(gps, 1, __func__ ": fail\n");
	free(data);
	return -1;
}
