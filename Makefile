# $Id: Makefile,v 1.2 1999/04/26 01:12:59 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit.  Tested on a GPS III/III+
#
SUBDIR= lib gardump garload

.include <bsd.subdir.mk>
