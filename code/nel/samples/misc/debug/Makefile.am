#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= debug_8.vcproj main.dsp main.dsw

bin_PROGRAMS              = debug

debug_SOURCES = main.cpp

debugdir = $(datadir)/nel/samples/debug

AM_CXXFLAGS               = -I$(top_srcdir)/src 

debug_LDADD   = ../../../src/misc/libnelmisc.la 

# End of Makefile.am

