#
# $Id: Makefile.am,v 1.1 2005/04/13 12:37:26 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

bin_PROGRAMS              = chatserver chatclient

chatserver_SOURCES = server.cpp

chatclient_SOURCES = client.cpp kbhit.h kbhit.cpp

chatserverdir=$(datadir)/nel/samples/net/chat
chatserver_DATA=chat_service.cfg

chatclientdir=$(datadir)/nel/samples/net/chat
chatclient_DATA=client.cfg

AM_CXXFLAGS               = -DCHAT_DIR="\"$(chatclientdir)\"" -I$(top_srcdir)/src 

chatserver_LDADD   = ../../../src/misc/libnelmisc.la \
                     ../../../src/net/libnelnet.la 

chatclient_LDADD   = ../../../src/misc/libnelmisc.la \
                     ../../../src/net/libnelnet.la 

# End of Makefile.am
