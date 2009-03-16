# $Id$
#
# gardump: program to dump waypoints, routes, and tracks from a garmin
# gps unit.

include ../GNUmakefile.inc

gardump: gardump.c
	gcc $(CFLAGS) gardump.c -L../lib -lgarmin -o gardump
clean:
	rm -f gardump
install:
	install gardump.1 $(MANDIR)/man1/
	install gardump   $(BINDIR)/
