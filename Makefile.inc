# $snafu: Makefile.inc,v 1.5 2001/05/02 00:34:51 marc Exp $
#
# Default installation directories
#
PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/cat

# Name the default serial port to talk to the gps.
#
SERIAL_PORT?=	-DDEFAULT_PORT=\"/dev/gps\"

# Program version
#
VERSION?=	-DVERSION=\"1.3\"

# Operation system
# Possible variants are
#  0 for BSD and
#  1 for Linux
# see lib/gps1.c file.
#
OS? = -DOS=0

# Type of GPS unit
# possible choises are
# 0   for Garmin GPS III/III+
# 1   for Garmin GPS 12/12XL
#
GPS? = -DGPS=0

# C options
#
COPTS+= -g -I${.CURDIR}/../lib
COPTS+=	-Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
COPTS+= ${SERIAL_PORT} ${VERSION} ${OS} ${GPS}

# Figure out where the library lives for proper dependencies
#
LIBGARMIN!=	cd $(.CURDIR)/../lib; \
		printf "xxx:\n\techo \$${.OBJDIR}/libgarmin.a\n" | \
		${MAKE} -r -s -f - xxx | grep libgarmin

# Clean up emacs backup files as part of a make clean
#
CLEANFILES+= ID *~


