# $snafu: Makefile.inc,v 1.7 2001/06/19 04:36:46 marc Exp $
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
VERSION?=	-DVERSION=\"1.4.2\"

# Which serial I/O code to use, possible choices are
#   BSD	   for BSD-based systems
#   Linux  for Linux
#
SIO_TYPE?=	-DSIO_TYPE=BSD

# C options
#
COPTS+= -g -I${.CURDIR}/../lib -O3
COPTS+= -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
COPTS+= ${SERIAL_PORT} ${VERSION} ${SIO_TYPE}

# Figure out where the library lives for proper dependencies
#
LIBGARMIN!=	cd $(.CURDIR)/../lib; \
		printf "xxx:\n\techo \$${.OBJDIR}/libgarmin.a\n" | \
		${MAKE} -r -s -f - xxx | grep libgarmin

# Clean up emacs backup files as part of a make clean
#
CLEANFILES+= ID *~ .depend
