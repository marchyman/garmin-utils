# $snafu: GNUmakefile,v 1.3 2003/04/25 03:40:26 marc Exp $
#
include ../GNUmakefile.inc

OBJS=		gps1.o gps2.o gpsdisplay.o gpsprod.o gpscap.o gpsdump.o\
                gpsprint.o gpsversion.o gpsformat.o gpsload.o strlcpy.o

libgarmin.a: $(OBJS)
	ar r libgarmin.a $(OBJS)
	ranlib libgarmin.a
clean:
	rm -f garmin.a *.o
gps1.o: gps1.c gpslib.h
gps2.o: gps2.c gpslib.h

gpscap.o: gpscap.c gpslib.h
gpsdisplay.o: gpsdisplay.c gpslib.h
gpsdump.o: gpsdump.c gpslib.h
gpsformat.o: gpsformat.c gpslib.h
gpsload.o:   gpsload.c gpslib.h
gpsprint.o:  gpsprint.c gpslib.h
gpsprod.o:   gpsprod.c gpslib.h
strlcpy.o: strlcpy.c
