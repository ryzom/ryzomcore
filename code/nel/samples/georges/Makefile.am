#
# $Id: Makefile.am,v 1.1 2005/04/04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

EXTRA_DIST = georges_sample_8.vcproj georges_sample.sln georges_sample.vcproj

bin_PROGRAMS              = georges

georges_SOURCES = main.cpp

georgesdir = $(datadir)/nel/samples/georges

georges_DATA = boolean.typ coolfilesinfo.dfn default.sample_config int.typ positiondata.dfn sample_config.dfn string.typ

AM_CXXFLAGS               = -DGF_DIR="\"$(georgesdir)\"" -I$(top_srcdir)/src


georges_LDADD   = ../../src/misc/libnelmisc.la \
                  ../../src/georges/libnelgeorges.la


# End of Makefile.am
