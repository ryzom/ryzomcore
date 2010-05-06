#
# $Id: Makefile.am,v 1.1 2005/04/04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

bin_PROGRAMS              = font

font_SOURCES = main.cpp

fontdir = $(datadir)/nel/samples/font
font_DATA = beteckna.ttf

AM_CXXFLAGS               = -DFONT_DIR="\"$(fontdir)\"" -I$(top_srcdir)/src 

font_LDADD   = ../../../src/misc/libnelmisc.la \
               ../../../src/3d/libnel3d.la


# End of Makefile.am

