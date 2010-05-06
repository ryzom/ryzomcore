#
# $Id: Makefile.am,v 1.1 2005-04-04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST		= demonel_8.sln demonel_8.vcproj demonel.rc demonel.sln demonel.vcproj icon1.ico datafiles

bin_PROGRAMS              = nel_sample_cegui

nel_sample_cegui_SOURCES = main.cpp NeLDriver.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src @CEGUI_CFLAGS@

nel_sample_cegui_LDADD   = ../../../src/misc/libnelmisc.la \
                           ../../../src/3d/libnel3d.la \
			   ../../../src/cegui/libnelceguirenderer.la \
			   @CEGUI_LIBS@


# End of Makefile.am

