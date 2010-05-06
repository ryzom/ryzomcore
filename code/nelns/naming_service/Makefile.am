#
# $Id: Makefile.am,v 1.5 2002/06/12 10:22:28 lecroart Exp $
#

MAINTAINERCLEANFILES   = Makefile.in

EXTRA_DIST		       = naming_service.cfg \
				 naming_service.dsp \
				 naming_service.vcproj \
				 naming_service_8.vcproj \
				 common.cfg

sbin_PROGRAMS			= naming_service

naming_servicedir		= ${pkgsysconfdir}
naming_service_DATA		= naming_service.cfg common.cfg

AM_CXXFLAGS			= -DNELNS_CONFIG="\"${pkgsysconfdir}\"" -DNELNS_STATE="\"${pkglocalstatedir}\"" -DNELNS_LOGS="\"${logdir}\""

naming_service_SOURCES = naming_service.cpp

# End of Makefile.am
