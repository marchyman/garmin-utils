# $Id: Makefile.inc,v 1.1 1998/05/12 05:01:14 marc Exp $
#
# Default installation directories
#
PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
MANDIR?=	${PREFIX}/man/cat

# Name the default serial port to talk to the gps.
#
SERIAL_PORT?=	-DDEFAULT_PORT=\"/dev/cuab\"

# C options
#
COPTS+= -g -I${.CURDIR}/../lib
COPTS+=	-Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
COPTS+= ${SERIAL_PORT}

# Figure out where the library lives for proper dependencies
#
LIBGARMIN!=	cd $(.CURDIR)/../lib; \
		printf "xxx:\n\techo \$${.OBJDIR}/libgarmin.a\n" | \
		${MAKE} -r -s -f - xxx | grep libgarmin

# Clean up emacs backup files as part of a make clean
#
CLEANFILES+= ID *~


