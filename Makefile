# $snafu: Makefile,v 1.5 2001/06/19 04:36:46 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit. Tested on III, III+, 12 and 12XL units.
#
SUBDIR= lib gardump garload

.include <bsd.subdir.mk>
