# $snafu: GNUmakefile,v 1.1 2001/05/02 00:34:50 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit.  Tested on III, 12 and 12XL units.
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
