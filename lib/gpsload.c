/*
 *	$snafu: gpsload.c,v 1.9 2003/04/11 21:17:15 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <err.h>
#include <stdio.h>

#include "gpslib.h"

/*
 * Garmin GPS load protocol
 *
 * dev1 -> dev2:	transfer begin
 * dev1 -> dev2:	transfer data
 * dev1 -> dev2:	transfer end
 */

/*
 * Send a start transfer.
 */
static int
start_load(gps_handle gps, int records)
{
	unsigned char buf[4];

	gps_printf(gps, 3, "start: load\n");
	buf[0] = p_xfr_begin;
	buf[1] = (unsigned char) records;
	buf[2] = (unsigned char) (records >> 8);
	return gps_send_wait(gps, buf, 3);
}

static int
do_load(gps_handle gps, struct gps_list_entry *entry)
{
	while (entry) {
		if (gps_send_wait(gps, entry->data, entry->data_len) != 1)
			return -1;
		entry = entry->next;
	}
	return 1;
}

static int
end_load(gps_handle gps, int type)
{
	unsigned char buf[4];

	gps_printf(gps, 3, "end: load\n");
	buf[0] = p_xfr_end;
	buf[1] = (unsigned char) type;
	buf[2] = (unsigned char) (type >> 8);
	return gps_send_wait(gps, buf, 3);
}

static int
cancel_load(gps_handle gps)
{
	unsigned char buf[4];

	gps_printf(gps, 3, "fail: load\n");
	buf[0] = p_xfr_end;
	buf[1] = (unsigned char) CMD_ABORT_XFR;
	buf[2] = 0;
	return gps_send_wait(gps, buf, 3);
}

/*
 * Load the lists specified.  Return 1 if upload successful,
 * -1 otherwise.
 */
int
gps_load(gps_handle gps, struct gps_lists * lists)
{
	while (lists) {
		if (start_load(gps, lists->list->count) != 1)
			return -1;
		if (do_load(gps, lists->list->head) != 1) {
			cancel_load(gps);
			return -1;
		} else {
			if (end_load(gps, lists->list->type) != 1)
				return -1;
		}
	}
	lists = lists->next;
	return 1;
}
