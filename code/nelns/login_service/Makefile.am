#
# $Id: Makefile.am,v 1.8 2002-09-16 14:50:07 lecroart Exp $
#

MAINTAINERCLEANFILES		= Makefile.in

EXTRA_DIST			= login_service.cfg \
				  login_service.dsp \
				  login_service.vcproj \
				  login_service_8.vcproj \
				  common.cfg

sbin_PROGRAMS			= login_service

login_servicedir		= ${pkgsysconfdir}
login_service_DATA		= login_service.cfg common.cfg
login_service_LDADD		= @MYSQL_LDFLAGS@

AM_CXXFLAGS			= -DNELNS_CONFIG="\"${pkgsysconfdir}\"" -DNELNS_STATE="\"${pkglocalstatedir}\"" -DNELNS_LOGS="\"${logdir}\"" @MYSQL_CFLAGS@

login_service_SOURCES = mysql_helper.cpp		\
			mysql_helper.h			\
			connection_client.cpp		\
			connection_client.h		\
			connection_web.cpp              \
                        connection_web.h                \
                        connection_ws.cpp               \
                        connection_ws.h                 \
                        login_service.cpp               \
                        login_service.h

# End of Makefile.am

