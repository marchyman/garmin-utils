# $snafu: Makefile,v 1.3 2001/05/02 00:34:51 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit. Tested on III, III+, 12 and 12XL units.
#
SUBDIR= lib gardump garload

CLEANFILES+=	*~

.include <bsd.subdir.mk>
