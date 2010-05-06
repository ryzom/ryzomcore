#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = build_smallbank.dsp

bin_PROGRAMS              = build_smallbank

build_smallbank_SOURCES   = build_smallbank.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

build_smallbank_LDADD     = ../../../src/misc/libnelmisc.la \
                            ../../../src/3d/libnel3d.la


# End of Makefile.am

