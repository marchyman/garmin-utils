# $Id: Makefile.inc,v 1.3 1998/08/28 20:40:27 marc Exp $
#
# Default installation directories
#
PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/cat

# Name the default serial port to talk to the gps.
#
SERIAL_PORT?=	-DDEFAULT_PORT=\"/dev/cua00\"

# Program version
#
VERSION?=	-DVERSION=\"1.1\"

# C options
#
COPTS+= -g -I${.CURDIR}/../lib
COPTS+=	-Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
COPTS+= ${SERIAL_PORT} ${VERSION}

# Figure out where the library lives for proper dependencies
#
LIBGARMIN!=	cd $(.CURDIR)/../lib; \
		printf "xxx:\n\techo \$${.OBJDIR}/libgarmin.a\n" | \
		${MAKE} -r -s -f - xxx | grep libgarmin

# Clean up emacs backup files as part of a make clean
#
CLEANFILES+= ID *~


