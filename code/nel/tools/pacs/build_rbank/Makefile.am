#
# $Id: Makefile.am,v 1.1 2002-05-13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = build_rbank.dsp

bin_PROGRAMS              = build_rbank 

build_rbank_SOURCES = build_rbank.cpp build_surf.cpp main.cpp prim_checker.cpp \
	surface_splitter.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

build_rbank_LDADD   =	../../../src/misc/libnelmisc.la	\
			../../../src/pacs/libnelpacs.la	\
			../../../src/ligo/libnelligo.la	\
			../../../src/3d/libnel3d.la


# End of Makefile.am

