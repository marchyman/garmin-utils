/*
 *	$snafu: gpsdump.c,v 1.7 2003/04/11 01:21:49 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gpslib.h"

/*
 * Garmin GPS device command and transfer protocols
 *
 * dev1 -> dev2:	command
 * dev2 -> dev1:	transfer begin
 * dev2 -> dev1:	transfer data
 * dev2 -> dev1:	transfer end
 */

/*
 * Issue a device command and wait for an ack.  Returns
 *	-1:	command failed
 *	0:	command naked
 *	1:	command acked.
 */
int
gps_cmd(gps_handle gps, enum gps_cmd_id cmd)
{
	unsigned char cmd_frame[4];
	int retries = 5;
	unsigned char *data = malloc(GPS_FRAME_MAX);

	if (data == NULL) {
		if (gps_debug(gps))
			warnx("no memory: gps command");
		return -1;
	}

	cmd_frame[0] = p_cmd_type;
	cmd_frame[1] = cmd;
	cmd_frame[2] = 0;
	
	if (gps_debug(gps) > 2)
		warnx("send: gps command %d", cmd);

	while (retries--) {
		if (gps_send_wait(gps, cmd_frame, 3) == 1) {

			/* command accepted, init data packet length and
			   kill any remaining retries */

			int datalen = GPS_FRAME_MAX;
			retries = 0;

			/* read until failure or end of transfer packet */

			while (gps_recv(gps, 2, data, &datalen) == 1) {
				gps_send_ack(gps, *data);
				gps_print(gps, cmd, data, datalen);
				if (*data == p_xfr_end || *data == p_utc_data) {
					break;
				}
				datalen = GPS_FRAME_MAX;
			}

			free(data);
			return 1;
		}
		if (gps_debug(gps) > 2)
			warnx("retry: gps command");
	}
	if (gps_debug(gps))
		warnx("fail: gps_cmd");
	free(data);
	return -1;
}
