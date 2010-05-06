#
#

MAINTAINERCLEANFILES         = Makefile.in

lib_LTLIBRARIES              = libnel_drv_fmod.la

libnel_drv_fmod_la_SOURCES =	buffer_fmod.cpp \
				buffer_fmod.h \
				listener_fmod.cpp \
				listener_fmod.h \
				music_channel_fmod.cpp \
				music_channel_fmod.h \
				sound_driver_fmod.cpp \
				sound_driver_fmod.h \
				source_fmod.cpp \
				source_fmod.h \
				stdfmod.cpp \
				stdfmod.h


AM_CXXFLAGS                  = -I$(top_srcdir)/src @FMOD_CFLAGS@

noinst_HEADERS		     = stdfmod.h

libnel_drv_fmod_la_LIBADD  = @FMOD_LIBS@

libnel_drv_fmod_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

