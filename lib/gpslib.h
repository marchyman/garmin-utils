/*
 * $snafu: gpslib.h,v 2.0 2003/10/06 19:13:52 marc Exp $
 *
 * Public Domain, 2003, Marco S Hyman <marc@snafu.org>
 */

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
#define p_cmd_type	(u_char) 10
#define p_xfr_end	(u_char) 12
#define p_utc_data	(u_char) 14
#define p_xfr_begin	(u_char) 27
#define p_rte_hdr	(u_char) 29
#define p_rte_wpt_data	(u_char) 30
#define p_trk_data	(u_char) 34
#define p_wpt_data	(u_char) 35
#define p_rte_link	(u_char) 98
#define p_trk_hdr	(u_char) 99
#define p_cap		(u_char) 253
#define p_prod_rqst	(u_char) 254
#define p_prod_resp	(u_char) 255

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
 * Magic headers used to flag the data types
 */
#define RTE_HDR	"[routes"
#define TRK_HDR	"[tracks"
#define WPT_HDR	"[waypoints"


/*
 * Known GPS data packet types
 */
#define D100		100
#define D101		101
#define D102		102
#define D103		103
#define D104		104
#define D105		105
#define D106		106
#define D107		107
#define D108		108
#define D109		109
#define D200		200
#define D201		201
#define D202		202
#define D210		210
#define D300		300
#define D301		301
#define D310		310

/*
 * list of data/length
 */
struct gps_list_entry {
	struct gps_list_entry *next;	/* next item in list */
	u_char *data;			/* pointer to data to send */
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
void	gps_display(char, const u_char *, int);
struct gps_lists *gps_format(gps_handle, FILE *);
int	gps_get_rte_hdr_type(gps_handle);
int	gps_get_rte_lnk_type(gps_handle);
int	gps_get_rte_wpt_type(gps_handle);
int	gps_get_trk_hdr_type(gps_handle);
int	gps_get_trk_type(gps_handle);
int	gps_get_wpt_type(gps_handle);
int	gps_load(gps_handle, struct gps_lists *);
gps_handle gps_open(const char *, int);
int	gps_print(gps_handle, enum gps_cmd_id, const u_char *, int);
void	gps_printf(gps_handle, int, const char *, ...)
	__attribute__((__format__(__printf__,3,4)));
int	gps_product(gps_handle, int *, int *, char **);
int	gps_protocol_cap(gps_handle);
int	gps_read(gps_handle, u_char *, int);
int	gps_recv(gps_handle, int, u_char *, int *);
int	gps_send(gps_handle, const u_char *, int);
int	gps_send_ack(gps_handle, u_char);
int	gps_send_nak(gps_handle, u_char);
int	gps_send_wait(gps_handle, const u_char *, int, int);
void	gps_set_rte_hdr_type(gps_handle, int);
void	gps_set_rte_lnk_type(gps_handle, int);
void	gps_set_rte_wpt_type(gps_handle, int);
void	gps_set_trk_hdr_type(gps_handle, int);
void	gps_set_trk_type(gps_handle, int);
void	gps_set_wpt_type(gps_handle, int);
int	gps_version(gps_handle);
int	gps_wait(gps_handle, u_char, int);
int	gps_write(gps_handle, const u_char *, int);

/*
 * What to do?  The strlcpy() code is provided for versions of Linux which 
 * do not have strlcpy.  But that means that the prototype is not in <string.h>
 * and the strict compiler options for gcc treat no prototype as an error 
 * Solution: Add a prototype here.
 */
#ifdef LINUX
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

