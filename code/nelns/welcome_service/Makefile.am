#
# $Id: Makefile.am,v 1.7 2006/05/31 12:14:16 boucher Exp $
#

MAINTAINERCLEANFILES		= Makefile.in

EXTRA_DIST			= welcome_service.cfg \
				  welcome_service.dsp \
				  welcome_service.vcproj \
				  welcome_service_8.vcproj \
				  common.cfg

sbin_PROGRAMS			= welcome_service

welcome_servicedir		= ${pkgsysconfdir}
welcome_service_DATA		= welcome_service.cfg common.cfg

AM_CXXFLAGS			= -DNELNS_CONFIG="\"${pkgsysconfdir}\"" -DNELNS_STATE="\"${pkglocalstatedir}\"" -DNELNS_LOGS="\"${logdir}\""

welcome_service_SOURCES		= welcome_service.cpp \
			  	  welcome_service_itf.cpp

# End of Makefile.am
