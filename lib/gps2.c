/*
 * $snafu: gps2.c,v 1.16 2003/09/27 05:50:33 marc Exp $
 *
 * Public Domain, 2001, Marco S Hyman <marc@snafu.org>
 */

#include <sys/types.h>

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
static u_char *
gps_frame(const u_char * buf, int *cnt)
{
	int sum = 0;
	int ix = 0;
	u_char *work = malloc(2 * *cnt + 10);

	if (! work) {
		warn( "%s: no memory", __func__);
		return 0;
	}

	/* start with a dle */
	work[ix++] = dle;

	/* record type, add to checksum */
	sum += *buf;
	work[ix++] = *buf++;
	*cnt -= 1;

	/* data length, escape if len == dle.  Add len to checksum */
	work[ix] = (u_char) *cnt;
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
	work[ix] = (u_char) (-sum);
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
gps_send(gps_handle gps, const u_char *buf, int cnt)
{
	int ok = -1;
	int len = cnt;
	u_char *data = gps_frame(buf, &len);

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
gps_send_ack(gps_handle gps, u_char type)
{
	u_char buf[4];

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
gps_send_nak(gps_handle gps, u_char type)
{
	u_char buf[4];

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
gps_recv(gps_handle gps, int to, u_char *buf, int * cnt)
{
	int dle_seen;
	int etx_seen;
	int sum;
	int len;
	int rlen = -1;
	u_char *ptr;
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
			gps_printf(gps, 2, "%s: sync error\n", __func__);
			return -1;
		case 0:
			gps_printf(gps, 2, "%s: timeout\n", __func__);
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
				gps_printf(gps, 2, "%s: frame error\n", __func__);
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
				gps_printf(gps, 1, "%s: bad frame len, "
					   "%d expected, %d received\n",
					   __func__, rlen, len);
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
				   "%s: frame too large for %d byte buffer\n",
				   __func__, *cnt);
			return -1;
		}
	}
}

/*
 * Wait for a response for a particular packet type, return
 *	1 = ack
 *	0 = nak
 *	-1 = other
 */
int
gps_wait(gps_handle gps, u_char typ, int timeout)
{
	u_char *response = malloc(GPS_FRAME_MAX);
	int result = -1;
	int retries = 3;
	int resplen;

	if (response)
		do {
			resplen = GPS_FRAME_MAX;
			if (gps_recv(gps, timeout, response, &resplen) != 1)
				break;
			if (resplen > 2)
				switch (response[0]) {
				case ack:
					if (response[1] == typ)
					        result = 1;
					break;
				case nak:
					if (response[1] == typ)
						result = 0;
					break;
				}
		} while (result == -1 && retries--);

	if (response)
		free(response);

	return result;
}

/*
 * send a frame and wait for an ack/nak.  If nak'd re-send the frame.
 * Return 1 if all ok, -1 if frame can not be sent/is not acknowledged,
 * or 0 if frame is always nak'd.
 */
int
gps_send_wait(gps_handle gps, const u_char *buf, int cnt, int timeout)
{
	int retries = 3;
	int ok = -1;
	int len = cnt;
	u_char *data = gps_frame(buf, &len);

	if (data) {
		if (gps_debug(gps) >= 4)
			gps_display('}', buf, cnt);
		do {
			if (gps_write(gps, data, len) == 1)
				ok = gps_wait(gps, *buf, timeout);
		} while (ok == 0 && retries--);
	}
	if (data)
		free(data);

	return ok;
}
