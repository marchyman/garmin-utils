# $Id: Makefile,v 1.1 1998/05/12 05:01:16 marc Exp $
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
