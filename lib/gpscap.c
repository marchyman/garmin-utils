/*
 *	$snafu: gpscap.c,v 1.7 2003/04/10 20:50:22 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "gpslib.h"

/*
 * Garmin GPS protocol capability application protocol
 *
 * gps -> host:	protocol array
 *
 *    tag:value	Protocol
 *
 *	P:0	physical protcol 0
 *	L:1	link protocol 1
 *	A:10	device comm protocol 1
 *	A:100	waypoint transfer protocol
 *	D:100	D100 records used for waypoint transfer
 *	A:200	route transfer protocol
 *	D:200	type D200 for D0 during route transfer
 *	D:100	type D100 for D1 during route transfer
 *	A:300	track log transfer protocol
 *	D:300	type D300 for D0 during track log transfer
 *	A:500	almanac transfer protocol
 *	D:500	type D500 for D0 during almanac transfer
 */

/*
 * See if the gps unit will send the supported protocol array.
 * Garmin says that some products will send this immediatly
 * following the product data protocol.  We'll wait about
 * 5 seconds for the data.  If it is not found assume it is
 * not supported.
 *
 * procedure returns -1 on error, otherwise 0.
 */
int
gps_protocol_cap(gps_handle gps)
{
	int retries = 5;
	unsigned char *data = malloc(GPS_FRAME_MAX);
	int datalen;
	int pntr;
	int tag;
	int val;

	if (! data) {
		if (gps_debug(gps))
			warnx("no memory: protocol capabilities");
		return -1;
	}
	if (gps_debug(gps) > 2)
		warnx("recv: protocol capabilities");
	while (retries--) {
		datalen = GPS_FRAME_MAX;
		switch (gps_recv(gps, 3, data, &datalen)) {
		case -1:
		case 0:
			goto done;
		case 1:
			if (data[0] == p_cap) {
				int phys = -1;
				int link = -1;
				int app = -1;
				int dat = -1;
				int found = 0;

				for (pntr = 1; pntr + 5 < datalen; pntr += 3) {
					tag = data[pntr];
					val = data[pntr + 1] +
						(data[pntr + 2] << 8);
					switch (tag) {
					case 'P':
						phys = val;
						link = app = dat = -1;
						break;
					case 'L':
						link = val;
						app = dat = -1;
						break;
					case 'A':
						app = val;
						dat = -1;
						break;
					case 'D':
						gps_set_capability(gps, phys,
								   link, app,
								   ++dat, val);
						break;
					default:
						warnx("unknown capability "
						      "tag %d\n", tag);
						break;
					}
					if (app == 100 && tag == 'D' &&
					    dat == 0 && !found) {
						gps_set_wpt_type(gps, val);
						found = 1;
						if (gps_debug(gps) > 1)
							warnx("waypoint packet "
							      "type is %d", val);
					}
				}
				gps_send_ack(gps, *data);
				free(data);
				if (gps_debug(gps) > 2)
					warnx("rcvd: protocol capabilities");
				return 0;
	    }
	    gpsSendNak(gps, *data);
	    if (gpsDebug(gps) > 2) {
		warnx("retry: protocol capabilities ");
	    }
	}
    }
 done:
    free(data);
    return -1;
}


void
gps_set_capability(gps_handle gps, int phys, int link, int app,
		   int dat, int typ)
{
	if (phys == 0) {
		/* common stuff between the two link protocols */
		if (link == 1 || link == 2) {
			switch (dat) {
			case 0:
				switch (app) {
				case D100:
					gps_set_wpt_wpt_type(gps, typ);
					break;
				case D200:
					gps_set_rte_rte_rdr(gps, typ);
					break;
				case D201:
					gps_set_rte_rte_hdr(gps, typ);
					break;
				case D300:
					gps_set_trk_trk_type(gps, typ);
					break;
				case D301:
					gps_set_trk_trk_hdr(gps, typ);
					break;
				default:
					/* unknown app (dat 0)*/
					;;;
					break;
				}
				break;

			case 1:
				switch (app) {
				case D200:
					gps_set_rte_wpt_type(gps, typ);
					break;
				case D201:
					gps_set_rte_wpt_type(gps, typ);
					gps_set_rte_rte_link(gps, typ);
					break;
				case D301:
					gps_set_trk_trk_type(gps, typ);
					break;
				default:
					/* unknown app (dat 1) */
					;;;
					break;
				}
				break;
			default:
				/* unknown dat */
				;;;
				break;
			}
		}
	}
}
