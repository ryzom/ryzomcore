#
# $Id: Makefile.am,v 1.1 2002-05-13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = make_sheet_id.dsp make_sheet_id.dsw

bin_PROGRAMS              = make_sheet_id 

make_sheet_id_SOURCES = make_sheet_id.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

make_sheet_id_LDADD   =	../../../src/misc/libnelmisc.la


# End of Makefile.am

