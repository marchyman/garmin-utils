# $snafu: Makefile,v 1.8 2003/06/12 16:44:59 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit.
#
SUBDIR= lib gardump garload

cleandir: _SUBDIRUSE
	rm -f ${.CURDIR}/TAGS ${.CURDIR}/ID ${.CURDIR}/*~

etags:	ETAGS
ETAGS:
	cd ${.CURDIR} && etags `find . -name '*.[chsS]' -print`

.PHONY: prep
prep:
	@cd ${.CURDIR} && ${MAKE} cleandir && \
	 (find . -name obj -type l -print | xargs rm) && \
	 ${MAKE} etags && mkid && ${MAKE} obj && ${MAKE} depend

.PHONY: todo
todo:
	@cd ${.CURDIR} && grep -nr ';;;' * | grep -v '.*~'

.include <bsd.subdir.mk>
