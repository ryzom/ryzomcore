#
#

MAINTAINERCLEANFILES         = Makefile.in

EXTRA_DIST			= driver_openal.def driver_openal.dsp driver_openal.vcproj

lib_LTLIBRARIES              = libnel_drv_openal.la

libnel_drv_openal_la_SOURCES = buffer_al.cpp               \
                               buffer_al.h                 \
                               listener_al.cpp             \
                               listener_al.h               \
                               sound_driver_al.cpp         \
                               sound_driver_al.h           \
                               source_al.cpp               \
                               source_al.h			\
                               stdopenal.h

AM_CXXFLAGS                  = -I$(top_srcdir)/src @OPENAL_CFLAGS@ @OGG_CFLAGS@ @VORBIS_CFLAGS@

noinst_HEADERS		     = stdopenal.h

libnel_drv_openal_la_LIBADD  = @OPENAL_LIBS@ @OGG_LIBS@ @VORBIS_LIBS@

libnel_drv_openal_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

