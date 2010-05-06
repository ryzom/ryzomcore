#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = config.cfg \
                            ig_lighter.dsp

bin_PROGRAMS              = ig_lighter

ig_lighter_SOURCES        = ig_lighter.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

ig_lighter_LDADD          = ../../../src/misc/libnelmisc.la    \
                            ../../../src/3d/libnel3d.la        \
                            ../../../src/pacs/libnelpacs.la    \
                            ../ig_lighter_lib/libig_lighter.la


# End of Makefile.am

