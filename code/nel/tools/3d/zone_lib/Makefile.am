#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES         = Makefile.in

noinst_LTLIBRARIES           = libzone.la

libzone_la_SOURCES           = zone_utility.cpp

noinst_HEADERS               = zone_utility.h

AM_CXXFLAGS                  = -I$(top_srcdir)/src

libzone_la_LIBADD            = ../../../src/misc/libnelmisc.la \
                               ../../../src/3d/libnel3d.la


# End of Makefile.am

