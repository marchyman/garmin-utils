# $snafu: GNUmakefile.inc,v 1.1 2001/05/02 00:34:51 marc Exp $
#
# Default installation directories
#
PREFIX =  /usr/local
BINDIR =  ${PREFIX}/bin
MANDIR =  ${PREFIX}/man

# Name of GPS device
#
CFLAGS = -DDEFAULT_PORT=\"/dev/gps\"

# Operation system
# Possible variants are 
#  0 for BSD and 
#  1 for Linux
# see lib/gps1.c file.
#
CFLAGS += -DOS=1

# Type of GPS unit
# possible choises are
# 0   for Garmin GPS III
# 1   for Garmin GPS 12/12XL
#
CFLAGS += -DGPS=1


# Program version
#
CFLAGS += -DVERSION=\"1.3\"

# Other C flags
#
CFLAGS += -I../lib
CFLAGS += -Wall -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Werror
