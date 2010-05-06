#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES         = Makefile.in

EXTRA_DIST                   = ig_lighter_lib.dsp

noinst_LTLIBRARIES           = libig_lighter.la

libig_lighter_la_SOURCES     = ig_lighter_lib.cpp

noinst_HEADERS               = ig_lighter_lib.h

AM_CXXFLAGS                  = -I$(top_srcdir)/src

libig_lighter_la_LIBADD      = ../../../src/misc/libnelmisc.la \
                               ../../../src/3d/libnel3d.la     \
                               ../../../src/pacs/libnelpacs.la


# End of Makefile.am

