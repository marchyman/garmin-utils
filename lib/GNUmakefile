# $snafu: GNUmakefile,v 1.1 2001/05/02 00:34:52 marc Exp $
#
include ../GNUmakefile.inc

OBJS=		gps1.o gps2.o gpsdisplay.o gpsprod.o gpscap.o gpsdump.o\
                gpsprint.o gpsversion.o gpsformat.o gpsload.o

libgarmin.a: $(OBJS)
	ar r libgarmin.a $(OBJS)
	ranlib libgarmin.a
clean:
	rm -f garmin.a *.o
gps1.o: gps1.c gps1.h gpsproto.h gpsdisplay.h
gps2.o: gps2.c gps1.h gps2.h gpsproto.h gpsdisplay.h

gpscap.o: gpscap.c gpscap.h gps1.h gps2.h gpsproto.h
gpsdisplay.o: gpsdisplay.c gpsdisplay.h
gpsdump.o: gpsdump.c gpsdump.h gps1.h gps2.h gpsproto.h gpsprint.h
gpsformat.o: gpsformat.c gpsformat.h gps1.h gpsproto.h
gpsload.o:   gpsload.c gpsload.h gps1.h gps2.h gpsproto.h gpsformat.h
gpsprint.o:  gpsprint.c gpsprint.h gpsdump.h gps1.h gpsproto.h
gpsprod.o:   gpsprod.c gpsprod.h gps1.h gps2.h gpsproto.h
