#
# $Id: Makefile.am,v 1.1 2005/04/04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES      = Makefile.in

bin_PROGRAMS              = sound_sources

sound_sources_SOURCES = main.cpp

AM_CXXFLAGS               = -I$(top_srcdir)/src 

sound_sources_LDADD   = ../../src/misc/libnelmisc.la \
                        ../../src/sound/libnelsnd.la \
                        ../../src/sound/driver/libnelsnd_lowlevel.la \
                        ../../src/sound/driver/fmod/libnel_drv_fmod.la \
			../../src/ligo/libnelligo.la \
			../../src/georges/libnelgeorges.la \
                        ../../src/3d/libnel3d.la


# End of Makefile.am

