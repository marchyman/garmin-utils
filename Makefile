# $snafu: Makefile,v 1.4 2001/05/03 16:43:41 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit. Tested on III, III+, 12 and 12XL units.
#
SUBDIR= lib gardump garload

# Clean up emacs backup files as part of a make clean
#
CLEANFILES+= ID *~

.include <bsd.subdir.mk>
