#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = zone_lighter.dsp

bin_PROGRAMS              = zone_lighter

zone_lighter_SOURCES      = zone_lighter.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

zone_lighter_LDADD        =	../../../src/misc/libnelmisc.la		\
				../../../src/3d/libnel3d.la		\
				../../../src/georges/libnelgeorges.la	\
				../zone_lib/libzone.la


# End of Makefile.am

