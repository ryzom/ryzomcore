#
# $Id: Makefile.am,v 1.1 2002-05-13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = disp_sheet_id.dsp disp_sheet_id_8.vcproj disp_sheet_id.dsw disp_sheet_id.sln disp_sheet_id.vcproj

bin_PROGRAMS              = disp_sheet_id 

disp_sheet_id_SOURCES = main.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

disp_sheet_id_LDADD   =	../../../src/misc/libnelmisc.la


# End of Makefile.am

