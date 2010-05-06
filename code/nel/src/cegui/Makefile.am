#
# $Id: Makefile.am,v 1.7 2002-06-10 17:02:05 cado Exp $
#

MAINTAINERCLEANFILES     = Makefile.in

lib_LTLIBRARIES          = libnelceguirenderer.la

libnelceguirenderer_la_SOURCES = nelrenderer.cpp nelresourceprovider.cpp neltexture.cpp

AM_CXXFLAGS         	 = -I$(top_srcdir)/src @CEGUI_CFLAGS@

libnelceguirenderer_la_LIBADD  = -lc -lpthread @CEGUI_LIBS@

libnelceguirenderer_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

