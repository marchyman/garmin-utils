/*
 *	$snafu: gpsformat.h,v 1.5 2001/12/16 00:56:16 marc Exp $
 *
 *	Placed in the Public Domain by Marco S. Hyman
 */

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
GpsLists * gpsFormat( GpsHandle gps, FILE * stream );

