# $snafu: Makefile,v 1.6 2003/04/14 21:08:49 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit. Tested on III, III+, 12 and 12XL units.
#
SUBDIR= lib gardump garload

cleandir: _SUBDIRUSE
	rm -f ${.CURDIR}/TAGS ${.CURDIR}/ID ${.CURDIR}/*~

.include <bsd.subdir.mk>
