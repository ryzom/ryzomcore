#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= command_8.vcproj main.dsp main.dsw

bin_PROGRAMS              = command

command_SOURCES = main.cpp

commanddir = $(datadir)/nel/samples/command

AM_CXXFLAGS               = -I$(top_srcdir)/src 

command_LDADD   = ../../../src/misc/libnelmisc.la 

# End of Makefile.am

