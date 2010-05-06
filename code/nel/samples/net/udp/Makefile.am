#
# $Id: Makefile.am,v 1.1 2005-04-13 12:37:26 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= main.dsw main.sln bench_service_8.vcproj bench_service.vcproj bench_service.dsp client_8.vcproj client.dsp client.vcproj readme.txt

bin_PROGRAMS              = udp_bench_service udp_bench_client

udp_bench_service_SOURCES = bench_service.cpp receive_task.cpp receive_task.h

udp_bench_client_SOURCES = client.cpp graph.cpp graph.h simlag.cpp simlag.h

noinst_HEADERS		= receive_task.h graph.h simlag.h

udp_bench_servicedir=$(datadir)/nel/samples/net/udp
udp_bench_service_DATA= bench_service.cfg

udp_bench_clientdir=$(datadir)/nel/samples/net/udp
udp_bench_client_DATA=client.cfg

AM_CXXFLAGS               = -DUDP_DIR="\"$(udp_bench_clientdir)\"" -I$(top_srcdir)/src 

udp_bench_service_LDADD   = ../../../src/misc/libnelmisc.la \
                     ../../../src/net/libnelnet.la 

udp_bench_client_LDADD   = ../../../src/misc/libnelmisc.la \
                     ../../../src/net/libnelnet.la 

# End of Makefile.am

