# $snafu: GNUmakefile.inc,v 1.2 2001/05/03 16:43:41 marc Exp $
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
#   0 for BSD-based systems
#   1 for Linux
#
CFLAGS+=	-DSIO_TYPE=1

# Model of the GPS unit, possible choices are
#   0 for Garmin GPS III/III+
#   1 for Garmin GPS 12/12XL/48
#
CFLAGS+=	-DGPS=0

# Program version
#
CFLAGS+=	-DVERSION=\"1.4\"

# Other C flags
#
CFLAGS+= -I../lib
CFLAGS+= -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
