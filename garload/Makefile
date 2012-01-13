# garload: program to load waypoints, routes, and tracks to a garmin
# gps unit.
#

PROG=	garload
DPADD+=	${LIBGARMIN}

.include <bsd.prog.mk>

.if exists(${.CURDIR}/../lib/${__objdir})
LDADD+=		-L${.CURDIR}/../lib/${__objdir} -lgarmin
.else
LDADD+=		-L${.CURDIR}/../lib -lgarmin
.endif
