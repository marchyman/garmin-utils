# $snafu: GNUmakefile,v 2.0 2003/10/06 19:10:41 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit.
#

all: LIB GARDUMP GARLOAD

LIB:
	${MAKE} -C lib
GARDUMP:
	${MAKE} -C gardump
GARLOAD:
	${MAKE} -C garload

clean:
	${MAKE} -C garload clean
	${MAKE} -C gardump clean
	${MAKE} -C lib     clean
