#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= main.dsp main.dsw strings_8.vcproj

bin_PROGRAMS              = strings

strings_SOURCES = main.cpp

stringsdir = $(datadir)/nel/samples/strings

AM_CXXFLAGS               = -I$(top_srcdir)/src 

strings_LDADD   = ../../../src/misc/libnelmisc.la 

# End of Makefile.am

