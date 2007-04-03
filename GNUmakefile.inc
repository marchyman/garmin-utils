# $snafu: GNUmakefile.inc,v 2.5 2007/04/03 17:48:58 marc Exp $
#
# Default installation directories
#
PREFIX =  /usr/local
BINDIR =  ${PREFIX}/bin
LIBDIR =  ${PREFIX}/lib
MANDIR =  ${PREFIX}/man

# Name of GPS device
#
CFLAGS+=	-DDEFAULT_PORT=\"/dev/gps\"

# Which serial I/O code to use, possible choices are
#   BSD    for BSD-based systems
#   Linux  for Linux
#
CFLAGS+=	-DSIO_TYPE=Linux

# Flag to use Linux-specific alternatives:
#	memcpy() lives in <string.h>,
#	location of <endian.h>
#	prototyping for strlcpy()
#
CFLAGS+=	-DLINUX

# Program version
#
CFLAGS+=	-DVERSION=\"2.5\"

# Other C flags
#
CFLAGS+= -I../lib
CFLAGS+= -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror

