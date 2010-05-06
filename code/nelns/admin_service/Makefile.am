#
# $Id: Makefile.am,v 1.7 2002/11/13 16:59:55 coutelas Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

EXTRA_DIST            =	common.cfg \
			admin_service.cfg \
			admin_service.dsp \
			admin_service_8.vcproj \
			admin_service.vcproj

sbin_PROGRAMS         = admin_service

admin_servicedir		= ${pkgsysconfdir}
admin_service_DATA		= admin_service.cfg common.cfg
admin_service_LDADD		= @MYSQL_LDFLAGS@

AM_CXXFLAGS			= -DNELNS_CONFIG="\"${pkgsysconfdir}\"" -DNELNS_STATE="\"${pkglocalstatedir}\"" -DNELNS_LOGS="\"${logdir}\"" @MYSQL_CFLAGS@

admin_service_SOURCES = admin_service.cpp connection_web.cpp

# End of Makefile.am

