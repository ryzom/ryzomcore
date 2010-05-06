#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:58 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = zwelder.cfg \
                            zone_welder.dsp

bin_PROGRAMS              = zone_welder

zone_welder_SOURCES = internal_weld.cpp zone_welder.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

zone_welder_LDADD         = ../../../src/misc/libnelmisc.la \
                            ../../../src/3d/libnel3d.la     \
                            ../zone_lib/libzone.la


# End of Makefile.am

