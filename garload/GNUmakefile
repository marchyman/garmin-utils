# $Id$
#
# gardump: program to dump waypoints, routes, and tracks from a garmin
# gps unit.

include ../GNUmakefile.inc

garload: garload.c
	cc $(CFLAGS) garload.c -L../lib -lgarmin -o garload 
clean:
	rm -f garload
install:
	install garload.1 $(MANDIR)/man1/
	install garload   $(BINDIR)/
