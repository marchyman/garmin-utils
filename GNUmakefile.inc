# $snafu: GNUmakefile.inc,v 1.7 2002/09/10 00:18:42 marc Exp $
#
# Default installation directories
#
PREFIX =  /usr/local
BINDIR =  ${PREFIX}/bin
MANDIR =  ${PREFIX}/man

# Name of GPS device
#
CFLAGS+=	-DDEFAULT_PORT=\"/dev/gps\"

# Which serial I/O code to use, possible choices are
#   BSD    for BSD-based systems
#   Linux  for Linux
#
CFLAGS+=	-DSIO_TYPE=Linux

# Program version
#
CFLAGS+=	-DVERSION=\"1.8\"

# Other C flags
#
CFLAGS+= -I../lib
CFLAGS+= -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
