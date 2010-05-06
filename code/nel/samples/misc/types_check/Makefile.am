#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= main.dsp main.dsw types_check_8.vcproj

bin_PROGRAMS              = types_check

types_check_SOURCES = main.cpp

types_checkdir = $(datadir)/nel/samples/types_check

AM_CXXFLAGS               = -I$(top_srcdir)/src 

types_check_LDADD   = ../../../src/misc/libnelmisc.la 

# End of Makefile.am

