# $Id: Makefile,v 1.1 1998/05/12 05:01:15 marc Exp $
#
# garload: program to load waypoints, routes, and tracks to a garmin
# gps unit.  Only tested on a GPS III
#

PROG=	garload
DPADD+=	${LIBGARMIN}

.include <bsd.prog.mk>

.if exists(${.CURDIR}/../lib/${__objdir})
LDADD+=		-L${.CURDIR}/../lib/${__objdir} -lgarmin
.else
LDADD+=		-L${.CURDIR}/../lib -lgarmin
.endif
