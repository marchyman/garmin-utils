# $snafu: GNUmakefile,v 1.2 2003/06/12 16:44:59 marc Exp $
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
