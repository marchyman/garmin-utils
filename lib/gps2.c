/*
 *	$snafu: gps2.c,v 1.8 2003/04/11 20:28:45 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "gpslib.h"

/*
 * Put application data into layer two frame format and return
 * a buffer with the formatted frame.  The size of the formated
 * frame is returned in the cnt parameter. Note: the buffer must be
 * freed.  Frame format is:
 *
 *	DLE
 *	record type, add to checksum
 *	data length, add to checksum, escape if DLE
 *	data, add to checksum, escape all DLE
 *	checksum, escape if DLE
 *	DLE
 *	ETX
 */
static unsigned char *
gps_frame(const unsigned char * buf, int *cnt)
{
	int sum = 0;
	int ix = 0;
	unsigned char *work = malloc(2 * *cnt + 10);

	if (! work) {
		warn("gps_frame: no memory");
		return 0;
	}

	/* start with a dle */
	work[ix++] = dle;

	/* record type, add to checksum */
	sum += *buf;
	work[ix++] = *buf++;
	*cnt -= 1;

	/* data length, escape if len == dle.  Add len to checksum */
	work[ix] = (unsigned char) *cnt;
	sum += work[ix];
	if (work[ix++] == dle)
		work[ix++] = dle;

	/* copy data (if any) to buffer adding to checksum and escaping
	   all dle characters */
	while ((*cnt)--) {
		sum += *buf;
		if (*buf == dle)
			work[ix++] = dle;
		work[ix++] = *buf++;
	}

	/* add the neg of the checksum. */
	work[ix] = (unsigned char) (-sum);
	if (work[ix++] == dle)
		work[ix++] = dle;

	/* add the final dle/etx */
	work[ix++] = dle;
	work[ix++] = etx;
	*cnt = ix;
	return work;
}
    
/*
 * Send a buffer containing layer 3 data using garmin layer 2 framing
 * to the device indicated by the gps_handle.  The first byte of the data
 * is assumed to be the garmin record type.
 *
 *	returns 1 if data sent and acknowledged, -1 if any errors occured.
 */
int
gps_send(gps_handle gps, const unsigned char *buf, int cnt)
{
	int ok = -1;
	int len = cnt;
	unsigned char *data = gps_frame(buf, &len);

	if (data) {
		if (gps_debug(gps) >= 4)
			gps_display('}', buf, cnt);
		ok = gps_write(gps, data, len);
		free(data);
	}
	return ok;
}

/*
 * Send an ack for the given packet type.  Return 1 if the packet
 * sent OK, othewise -1.
 */
int
gps_send_ack(gps_handle gps, unsigned char type)
{
	unsigned char buf[4];

	buf[0] = ack;
	buf[1] = type;
	buf[2] = 0;
	return gps_send(gps, buf, 3);
}

/*
 * Send a nak for the given packet type. Return 1 if the packet
 * sent OK, othewise -1.
 */
int
gps_send_nak(gps_handle gps, unsigned char type)
{
	unsigned char buf[4];

	buf[0] = nak;
	buf[1] = type;
	buf[2] = 0;
	return gps_send(gps, buf, 3);
}

/*
 * Receive a frame from the gps unit indicated by the gps_handle.
 * Data is put into buf for up to *cnt bytes.  *cnt is updated
 * with the number of bytes actually received.  The variable 'to'
 * specifies a timeout before the start of a message is received.
 * Use -1 to block.
 *
 * Function returns:
 *	1 - data received
 *	0 - timeout
 *	-1 - error
 */
#define READ_TO	10

int
gps_recv(gps_handle gps, int to, unsigned char * buf, int * cnt)
{
	int dle_seen;
	int etx_seen;
	int sum;
	int len;
	int rlen = -1;
	unsigned char *ptr;
	int stat;

	/* sync to the first DLE to come down the pike. */

	while (1) {
		do {
			stat = gps_read(gps, buf, to);
		} while ((stat == 1) && (*buf != dle));

		/* We have a timeout or a frame (or possibly the middle or
		   end of a packet). If a timeout return a -1, otherwise
		   prepare to receive the rest of the frame */

		switch (stat) {
		case -1:
			gps_printf(gps, 2, "sync error: gps recv\n");
			return -1;
		case 0:
			gps_printf(gps, 2, "timeout: gps recv\n");
			return 0;
		case 1:
			break;
		}

		/* start receiving characters into buf.  An end of buffer or
		   a DLE ETX sequence will terminate the reception.  Each
		   read is given a READ_TO second timeout -- if we time out
		   assume the gps died and return an error. */

		ptr = buf;
		dle_seen = 0;
		etx_seen = 0;
		sum = 0;
		len = 0;
		do {
			stat = gps_read(gps, ptr, READ_TO);
			if (stat != 1) {
				gps_printf(gps, 2, "frame error: gps recv\n");
				return -1;
			}
			if (dle_seen) {
				if (*ptr == etx) {
					etx_seen = 1;
					break;
				}
				dle_seen = 0;
			} else {
				if (*ptr == dle) {
					dle_seen = 1;
					continue;
				}
			}
			if ((rlen == -1) && (len == 1)) {
				/* this is the length byte, add it to the
				   checksum and save it, but do not keep it
				   in the buffer */ 
				sum += *ptr;
				rlen = *ptr;
			} else {
				sum += *ptr++;
				len += 1;
			}
		} while (len < *cnt);

		if (etx_seen) {
			/* subtract one from the length as we don't count the
			   checksum. */
			len -= 1;

			/* warn if the length is not the expected value.
			   Add in the packet type to the expected length. */
			rlen += 1;
			if (rlen != len)
				gps_printf(gps, 1, "bad frame len, "
					   "%d expected, %d received\n",
					   rlen, len);
			if ((sum & 0xff) == 0) {
				/* good checksum, update len rcvd and return */
				*cnt = len;
				if (gps_debug(gps) >= 4)
					gps_display('{', buf, len);
				return 1;
			} else {
				/* bad checksum -- try again */
				if (gps_debug(gps) >= 4)
					gps_display('!', buf, len);
				return -1;
			}
		} else {
			/* frame too large, return error */
			gps_printf(gps, 1,
				   "frame too large for %d byte buffer\n",
				   *cnt);
			return -1;
		}
	}
}

/*
 * send a frame and wait for an ack/nak.  If nak'd re-send the frame.
 * Return 1 if all ok, -1 if frame can not be sent/is not acknowledged.
 */
int
gps_send_wait(gps_handle gps, const unsigned char * buf, int cnt)
{
	int retry = 5;
	int ok = -1;
	int len = cnt;
	unsigned char *data = gps_frame(buf, &len);
	unsigned char *response = malloc(GPS_FRAME_MAX);
	int resplen;

	if (data && response) {
		if (gps_debug(gps) >= 4)
			gps_display('}', buf, cnt);
		while (retry--) {
			if (gps_write(gps, data, len) == 1) {
				resplen = GPS_FRAME_MAX;
				if (gps_recv(gps, 2, response, &resplen) == 1) {
					if ((resplen > 2) &&
					    (response[0] == ack) &&
					    (response[1] == *buf)) { 
						ok = 1;
						break;
					}
				}
				gps_printf(gps, 2, "retry: send and wait\n");
			}
		}
	}
	if (data)
		free(data);
	if (response)
		free(response);

	return ok;
}
