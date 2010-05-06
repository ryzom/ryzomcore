#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = panoply.cfg \
                            panoply_maker.dsp

bin_PROGRAMS              = panoply_maker

panoply_maker_SOURCES = color_modifier.cpp hls_bank_texture_info.cpp \
	panoply_maker.cpp

noinst_HEADERS            = color_mask.h \
                            color_modifier.h

AM_CXXFLAGS               = -I$(top_srcdir)/src 

panoply_maker_LDADD       = ../../../src/misc/libnelmisc.la


# End of Makefile.am

