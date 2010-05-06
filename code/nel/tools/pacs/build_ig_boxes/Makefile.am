#
# $Id: Makefile.am,v 1.1 2002-05-13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = build_ig_boxes.dsp

bin_PROGRAMS              = build_ig_boxes 

build_ig_boxes_SOURCES = main.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

build_ig_boxes_LDADD   =	../../../src/misc/libnelmisc.la	\
				../../../src/3d/libnel3d.la	\
				../../../src/pacs/libnelpacs.la


# End of Makefile.am

