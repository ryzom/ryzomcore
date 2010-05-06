#
# $Id: Makefile.am,v 1.1 2002-05-13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = build_indoor_rbank.dsp

bin_PROGRAMS              = build_indoor_rbank 

build_indoor_rbank_SOURCES = build_surfaces.cpp main.cpp mouline.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

build_indoor_rbank_LDADD   =	../../../src/misc/libnelmisc.la	\
				../../../src/pacs/libnelpacs.la	


# End of Makefile.am

