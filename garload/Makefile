# $snafu: Makefile,v 1.2 2001/05/02 00:34:52 marc Exp $
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
