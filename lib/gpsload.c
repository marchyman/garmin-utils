/*
 *	$snafu: gpsload.c,v 1.13 2003/04/17 23:35:18 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <sys/types.h>

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
	u_char buf[4];

	gps_printf(gps, 3, __func__ ": send\n");
	buf[0] = p_xfr_begin;
	buf[1] = (u_char) records;
	buf[2] = (u_char) (records >> 8);
	return gps_send_wait(gps, buf, 3, 2);
}

static int
do_load(gps_handle gps, struct gps_list_entry *entry)
{
	while (entry) {
		if (gps_send_wait(gps, entry->data, entry->data_len, 2) != 1)
			return -1;
		entry = entry->next;
	}
	return 1;
}

static int
end_load(gps_handle gps, int type)
{
	u_char buf[4];

	gps_printf(gps, 3, __func__ ": send\n");
	buf[0] = p_xfr_end;
	buf[1] = (u_char) type;
	buf[2] = (u_char) (type >> 8);
	return gps_send_wait(gps, buf, 3, 2);
}

static int
cancel_load(gps_handle gps)
{
	u_char buf[4];

	gps_printf(gps, 3, __func__ ": send\n");
	buf[0] = p_xfr_end;
	buf[1] = (u_char) CMD_ABORT_XFR;
	buf[2] = 0;
	return gps_send_wait(gps, buf, 3, 5);
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
		lists = lists->next;
	}
	return 1;
}
