#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= log_8.vcproj main.dsp main.dsw

bin_PROGRAMS              = log

log_SOURCES = main.cpp

logdir = $(datadir)/nel/samples/log

AM_CXXFLAGS               = -I$(top_srcdir)/src 

log_LDADD   = ../../../src/misc/libnelmisc.la 

# End of Makefile.am

