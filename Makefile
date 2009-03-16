# $Id$
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
	 (find . -name obj -type l -print | xargs -r rm) && \
	 ${MAKE} etags && mkid && ${MAKE} obj && ${MAKE} depend

.PHONY: todo
todo:
	@cd ${.CURDIR} && grep -nr ';;;' * | grep -v '.*~'

.include <bsd.subdir.mk>
