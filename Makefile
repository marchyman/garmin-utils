# $snafu: Makefile,v 1.7 2003/04/16 23:08:37 marc Exp $
#
# gardump/garload: programs to dump/load waypoints, routes, and tracks from
# a garmin gps unit. Tested on III, III+, 12 and 12XL units.
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
