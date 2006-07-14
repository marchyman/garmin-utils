# $snafu: Makefile.inc,v 2.3 2006/07/14 02:35:53 marc Exp $
#
# Default installation directories
#
PREFIX?=	/usr/local
BINDIR?=	${PREFIX}/bin
.if !empty(unix:M*OpenBSD.)
MANDIR?=	${PREFIX}/man/cat
.else
MANDIR?=	${PREFIX}/man
.endif

# Name the default serial port to talk to the gps
#
GPS_SERIAL_PORT?=	/dev/tty00

# Program version
#
VERSION?=	-DVERSION=\"2.3\"

# Which serial I/O code to use, possible choices are
#   BSD	   for BSD-based systems
#   Linux  for Linux
#
SIO_TYPE?=	-DSIO_TYPE=BSD

# C options
#
CFLAGS+= -g -I${.CURDIR}/../lib
CFLAGS+= -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
CFLAGS+= -DDEFAULT_PORT=\"${GPS_SERIAL_PORT}\" ${VERSION} ${SIO_TYPE}

# Figure out where the library lives for proper dependencies
#
LIBGARMIN!=	cd $(.CURDIR)/../lib; \
		printf "xxx:\n\techo \$${.OBJDIR}/libgarmin.a\n" | \
		${MAKE} -r -s -f - xxx | grep libgarmin

# Clean up emacs backup files as part of a make clean
#
CLEANFILES+= ${.CURDIR}/TAGS ${.CURDIR}/ID ${.CURDIR}/*~
