/*
 *	$Id: gpsformat.h,v 1.1 1998/05/12 23:13:05 marc Exp $
 *
 *	Copyright (c) 1998 Marco S. Hyman
 *
 *	Permission to copy all or part of this material for any purpose is
 *	granted provided that the above copyright notice and this paragraph
 *	are duplicated in all copies.  THIS SOFTWARE IS PROVIDED ``AS IS''
 *	AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 *	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *	FOR A PARTICULAR PURPOSE.
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

