# $Id: Makefile,v 1.1 1998/05/12 05:01:14 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit.  Only tested on a GPS III
#
SUBDIR= lib gardump garload

.include <bsd.subdir.mk>
