#
# $Id: Makefile.am,v 1.1 2005-04-13 12:37:26 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= main.dsw client.dsp client.cfg frontend_service.cfg frontend_service.dsp

bin_PROGRAMS              = nls_login_client nls_frontend_service

nls_frontend_service_SOURCES = frontend_service.cpp

nls_login_client_SOURCES = client.cpp

nls_frontend_servicedir=$(datadir)/nel/samples/net/udp
nls_frontend_service_DATA=frontend_service.cfg

nls_login_clientdir=$(datadir)/nel/samples/net/udp
nls_login_client_DATA=client.cfg

AM_CXXFLAGS               = -DLC_DIR="\"$(nls_login_clientdir)\"" -I$(top_srcdir)/src 

nls_frontend_service_LDADD   = ../../../src/misc/libnelmisc.la \
                     ../../../src/net/libnelnet.la 

nls_login_client_LDADD   = ../../../src/misc/libnelmisc.la \
                     ../../../src/net/libnelnet.la 

# End of Makefile.am

