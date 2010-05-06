#
# $Id: Makefile.am,v 1.1 2002/05/13 15:44:57 valignat Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST                = build.cfg \
                            build_coarse_mesh.dsp

bin_PROGRAMS              = build_coarse_mesh

build_coarse_mesh_SOURCES = build_coarse_mesh.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

build_coarse_mesh_LDADD   = ../../../src/misc/libnelmisc.la \
                            ../../../src/3d/libnel3d.la


# End of Makefile.am

