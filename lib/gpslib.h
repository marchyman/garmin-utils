/* $snafu: gpslib.h,v 1.3 2003/04/11 01:21:49 marc Exp $ */
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
#define p_cmd_type	10
#define p_xfr_end	12
#define p_utc_data	14
#define p_xfr_begin	27
#define p_rte_hdr	29
#define p_rte_wpt_data	30
#define p_trk_data	34
#define p_wpt_data	35
#define p_rte_link	98
#define p_trk_hdr	99
#define p_cap		253
#define p_prod_rqst	254
#define p_prod_resp	255

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
enum gps_cmd_id {
    CMD_ABORT_XFR = 0,
    CMD_RTE = 4,
    CMD_UTC = 5,
    CMD_TRK = 6,
    CMD_WPT = 7
};

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

/*
 * list of data/length
 */
struct gps_list_entry {
	struct gps_list_entry *next;	/* next item in list */
	unsigned char *data;		/* pointer to data to send */
	int data_len;			/* length of data to send */
};

/*
 * list head
 */
struct gps_list_head {
	int type;			/* protocol type */
	struct gps_list_entry *head;	/* first entry in list */
	struct gps_list_entry *tail;	/* last entry in list */
	int count;			/* number of entries */
};

/*
 * a list of list heads
 */
struct gps_lists {
	struct gps_lists *next;		/* next list */
	struct gps_list_head *list;	/* head of this list */
};

/*
 * opaque type to identify a gps state structure
 */
typedef void * gps_handle;

void	gps_close(gps_handle);
int	gps_cmd(gps_handle, enum gps_cmd_id);
int	gps_debug(gps_handle);
void	gps_display(char, const unsigned char *, int);
struct gps_lists *gps_format(gps_handle, FILE *);
int	gps_get_rte_hdr_type(gps_handle);
int	gps_get_rte_lnk_type(gps_handle);
int	gps_get_rte_wpt_type(gps_handle);
int	gps_get_trk_hdr_type(gps_handle);
int	gps_get_trk_type(gps_handle);
int	gps_get_wpt_type(gps_handle);
int	gps_load(gps_handle, struct gps_lists *);
gps_handle gps_open(const char *, int);
int	gps_print(gps_handle, enum gps_cmd_id, const unsigned char *, int);
int	gps_product(gps_handle, int *, int *, char **);
int	gps_protocol_cap(gps_handle);
int	gps_read(gps_handle, unsigned char *, int);
int	gps_recv(gps_handle, int, unsigned char *, int *);
int	gps_send(gps_handle, const unsigned char *, int);
int	gps_send_ack(gps_handle, unsigned char);
int	gps_send_nak(gps_handle, unsigned char);
int	gps_send_wait(gps_handle, const unsigned char *, int);
void	gps_set_rte_hdr_type(gps_handle, int);
void	gps_set_rte_lnk_type(gps_handle, int);
void	gps_set_rte_wpt_type(gps_handle, int);
void	gps_set_trk_hdr_type(gps_handle, int);
void	gps_set_trk_type(gps_handle, int);
void	gps_set_wpt_type(gps_handle, int);
int	gps_version(gps_handle);
int	gps_write(gps_handle, const unsigned char *, int);

