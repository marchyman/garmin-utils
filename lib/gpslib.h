/* $snafu: gpslib.h,v 1.1 2003/04/10 18:58:27 marc Exp $ */
/* PUBLIC DOMAIN April 2003 <marc@snafu.org> */

/*
 * ascii command characters
 */
#define etx		3
#define ack		6
#define dle		16
#define nak		21

/*
 * Known GPS protocol identifiers
 */
#define cmd_type	10
#define xfr_end		12
#define utc_data	14
#define xfr_begin	27
#define rte_hdr		29
#define rte_wpt_data	30
#define trk_data	34
#define wpt_data	35
#define rte_link	98
#define trk_hdr		99
#define proto_cap	253
#define prod_rqst	254
#define prod_resp	255

    /*
     * Raw input buffer -- read up to this many characters at a time
     */
#define GPS_BUF_LEN 512

    /*
     * Size of an application data buffer.  This is large enough to
     * hold the frame type plus the largest allowable frame.
     */
#define GPS_FRAME_MAX	256

    /*
     * Gps command (upload/download) types.
     */
typedef enum {
    CMD_ABORT_XFR = 0,
    CMD_RTE = 4,
    CMD_UTC = 5,
    CMD_TRK = 6,
    CMD_WPT = 7
} GpsCmdId;

    /*
     * Several GPS protocols
     */
#define	A100		100	/* waypoint transfer protocol */

    /*
     * Known GPS waypoint packet types
     */
#define D100		100
#define D103		103
#define D104		104
#define D108		108
#define D200		200
#define D201		201
#define D202		202
#define D300		300
#define D301		301


typedef void * gps_handle;

gps_handle gps_open(const char *, int);

void	gps_close(gps_handle);
int	gpsDebug(gps_handle);
int	gps_read(gps_handle, unsigned char *, int);
int	gps_write(gps_handle, const unsigned char *, int);
void	gps_set_wpt_type(gps_handle, int);
int	gps_get_wpt_type(gps_handle);
void	gps_set_capability(gps_handle, int, int, int, int, int);
int	gps_get_rte_rte_hdr(gps_handle);
int	gps_get_trk_trk_type(gps_handle);


    /*
     * Send a buffer containing layer 3 data using garmin layer 2 framing
     * to the device indicated by the gps_handle.
     *
     *	returns 1 if data sent and acknowledged, -1 if any errors occured.
     */
int gpsSend(gps_handle gps, const unsigned char * buf, int cnt);

    /*
     * Send an ack for the given packet type.  Return 1 if the packet
     * sent OK, othewise -1.
     */
int gpsSendAck(gps_handle gps, unsigned char type);

    /*
     * Send a nak for the given packet type. Return 1 if the packet
     * sent OK, othewise -1.
     */
int gpsSendNak(gps_handle gps, unsigned char type);

    /*
     * Receive a frame from the gps unit indicated by the gps_handle.
     * Data is put into buf for up to *cnt bytes.  *cnt is updated
     * with the number of bytes actually received.  to specifies
     * a timeout before the start of a message is received.  Use -1
     * to block.
     *
     * Function returns:
     *	1 - data received
     *	0 - timeout
     *	-1 - error
     */
int gpsRecv(gps_handle gps, int to, unsigned char * buf, int * cnt);

    /*
     * send a frame and wait for an ack/nak.  If nak'd re-send the frame.
     * Return 1 if all ok, -1 if frame can not be sent/is not acknowledged.
     */
int gpsSendWait(gps_handle gps, const unsigned char * buf, int cnt);


    /*
     * Garmin GPS protocol capability application protocol
     *
     * gps -> host:	protocol array
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
int gpsProtocolCap(gps_handle gps);



    /*
     * Write len bytes at buf to stderr.  The `direction' parameter is
     * intended to be '<' to show data received and '>' to show data
     * sent, but any character can be used.
     */
void gpsDisplay(char direction, const unsigned char * buf, int len);

    /*
     * Request the unit version and output to stdout.  Wait for a
     * protocol capabilities packet and consume it if seen.
     *
     *	-1:	command failed
     *	1:	command succeeded.
     */
int gpsVersion(gps_handle gps);

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
     * procedure returns -1 on error, otherwise 0.
     */
int gpsProduct(gps_handle gps, int * productId, int * softwareVersion,
		char ** productDescription);


    /*
     * Functions to `print' gps data.  The given packets are formatted
     * and written to stdout.  Formatting varies according to the
     * packet type.
     */
int gpsPrint(gps_handle gps, GpsCmdId cmd, const unsigned char * packet,
	      int len);

    /*
     * Garmin GPS load protocol
     *
     * dev1 -> dev2:	transfer begin
     * dev1 -> dev2:	transfer data
     * dev1 -> dev2:	transfer end
     */

    /*
     * Load the lists specified.  Return 1 if upload successful,
     * -1 otherwise.
     */
int gpsLoad(gps_handle gps, GpsLists * lists);

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
int gpsCmd(gps_handle gps, GpsCmdId cmd);

    /*
     * Convert a given file, assumed to be in the same format output
     * by gpsprint, to lists of gps records ready to upload to a gps
     * unit.
     */
typedef struct gpsListEntry {
    struct gpsListEntry * next;		/* next item in list */
    unsigned char * data;		/* pointer to data to send */
    int dataLen;			/* length of data to send */
} GpsListEntry;

typedef struct gpsListHead {
    int type;				/* protocol type */
    GpsListEntry * head;		/* first entry in list */
    GpsListEntry * tail;		/* last entryin list */
    int count;				/* number of entries */
} GpsListHead;

typedef struct gpsLists {
    struct gpsLists * next;		/* next list */
    GpsListHead * list;			/* head of this list */
} GpsLists;

    /*
     * Scan stdin, building GPS upload lists.  Return pointer to
     * GpsLists, if one or more lists created, otherwise return
     * a null pointer.
     *
     * The lists and each item from the list come from the heap and
     * should be released.
     */
GpsLists * gpsFormat(gps_handle gps, FILE * stream);

