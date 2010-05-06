#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = build_far_bank.dsp

bin_PROGRAMS              = build_far_bank

build_far_bank_SOURCES    = build_far_bank.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

build_far_bank_LDADD      = ../../../src/misc/libnelmisc.la \
                            ../../../src/3d/libnel3d.la


# End of Makefile.am

