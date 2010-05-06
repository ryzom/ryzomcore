#
#

MAINTAINERCLEANFILES          = Makefile.in

EXTRA_DIST			= fmod openal dsound

SUBDIRS                       = @SOUND_SUBDIRS@

noinst_LTLIBRARIES            = libnelsnd_lowlevel.la

libnelsnd_lowlevel_la_SOURCES = buffer.cpp                 \
                                buffer.h                   \
                                effect.h                   \
                                effect.cpp                 \
                                listener.cpp               \
                                listener.h                 \
                                sound_driver.cpp           \
                                sound_driver.h             \
                                source.cpp                 \
                                source.h

AM_CXXFLAGS                   = -I$(top_srcdir)/src

# End of Makefile.am

