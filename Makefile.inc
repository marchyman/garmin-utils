# $snafu: Makefile.inc,v 1.6 2001/05/03 16:43:41 marc Exp $
#
# Default installation directories
#
PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/cat

# Name the default serial port to talk to the gps
#
SERIAL_PORT?=	-DDEFAULT_PORT=\"/dev/gps\"

# Program version
#
VERSION?=	-DVERSION=\"1.4\"

# Which serial I/O code to use, possible choices are
#   0 for BSD-based systems
#   1 for Linux
#
SIO_TYPE?=	-DSIO_TYPE=0

# Model of the GPS unit, possible choices are
#   0 for Garmin GPS III/III+
#   1 for Garmin GPS 12/12XL/48
#
GPS?=		-DGPS=0

# C options
#
COPTS+= -g -I${.CURDIR}/../lib
COPTS+=	-Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
COPTS+= ${SERIAL_PORT} ${VERSION}

CFLAGS+= ${SIO_TYPE} ${GPS}

# Figure out where the library lives for proper dependencies
#
LIBGARMIN!=	cd $(.CURDIR)/../lib; \
		printf "xxx:\n\techo \$${.OBJDIR}/libgarmin.a\n" | \
		${MAKE} -r -s -f - xxx | grep libgarmin
