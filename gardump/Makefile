# $snafu: Makefile,v 1.2 2001/05/02 00:34:51 marc Exp $
#
# gardump: program to dump waypoints, routes, and tracks from a garmin
# gps unit.  Only tested on a GPS III
#

PROG=	gardump
DPADD+=	${LIBGARMIN}

.include <bsd.prog.mk>

.if exists(../lib/${__objdir})
LDADD+=	-L${.CURDIR}/../lib/${__objdir} -lgarmin
.else
LDADD+=	-L${.CURDIR}/../lib -lgarmin
.endif
