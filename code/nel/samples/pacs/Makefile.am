#
# $Id: Makefile.am,v 1.1 2005/04/04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

bin_PROGRAMS              = pacs_sample

pacs_sample_SOURCES = main.cpp object.cpp object.h

AM_CXXFLAGS               = -I$(top_srcdir)/src 

pacs_sample_LDADD   = ../../src/misc/libnelmisc.la \
                      ../../src/3d/libnel3d.la \
                      ../../src/pacs/libnelpacs.la


# End of Makefile.am

