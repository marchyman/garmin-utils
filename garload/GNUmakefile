# $snafu: GNUmakefile,v 1.1 2001/05/02 00:34:52 marc Exp $
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
