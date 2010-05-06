#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= configfile_8.vcproj main.dsp main.dsw

bin_PROGRAMS              = configfile

configfile_SOURCES = main.cpp

configfiledir = $(datadir)/nel/samples/configfile

configfile_DATA = simpletest.txt

AM_CXXFLAGS               = -DCF_DIR="\"$(configfiledir)\"" -I$(top_srcdir)/src 

configfile_LDADD   = ../../../src/misc/libnelmisc.la 


# End of Makefile.am

