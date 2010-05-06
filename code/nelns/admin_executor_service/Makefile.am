#
# $Id: Makefile.am,v 1.8 2004/03/02 09:10:50 cado Exp $
#

MAINTAINERCLEANFILES		= Makefile.in

EXTRA_DIST			= admin_executor_service.cfg \
				  admin_executor_service.dsp \
				  admin_executor_service_8.vcproj \
				  admin_executor_service.vcproj \
				  log_report.h \
				  common.cfg

sbin_PROGRAMS			= admin_executor_service

admin_executor_servicedir	= ${pkgsysconfdir}
admin_executor_service_DATA	= admin_executor_service.cfg common.cfg

AM_CXXFLAGS			= -DNELNS_CONFIG="\"${pkgsysconfdir}\"" -DNELNS_STATE="\"${pkglocalstatedir}\"" -DNELNS_LOGS="\"${logdir}\""

admin_executor_service_SOURCES = admin_executor_service.cpp \
				 log_report.cpp

# End of Makefile.am

