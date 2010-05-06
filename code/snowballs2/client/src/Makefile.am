#
# $Id: Makefile.am,v 1.10 2003-04-07 14:06:08 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS         = snowballs

snowballs_SOURCES    = animation.cpp                       \
                       animation.h                         \
                       camera.cpp                          \
                       camera.h                            \
                       commands.cpp                        \
                       commands.h                          \
                       compass.cpp                         \
                       compass.h                           \
                       configuration.cpp                   \
                       configuration.h                     \
                       entities.cpp                        \
                       entities.h                          \
                       game_time.cpp                       \
                       game_time.h                         \
                       graph.cpp                           \
                       graph.h                             \
                       interface.cpp                       \
                       interface.h                         \
                       internationalization.cpp            \
                       internationalization.h              \
                       landscape.cpp                       \
                       landscape.h                         \
                       lens_flare.cpp                      \
                       lens_flare.h                        \
                       mouse_listener.cpp                  \
                       mouse_listener.h                    \
                       network.cpp                         \
                       network.h                           \
                       pacs.cpp                            \
                       pacs.h                              \
                       physics.cpp                         \
                       physics.h                           \
                       radar.cpp                           \
                       radar.h                             \
                       snowballs_client.cpp                \
                       snowballs_client.h                  \
                       snowballs_config.h                  \
                       sound.cpp                           \
                       sound.h

snowballs_LDADD      = @FREETYPE_LIBS@ @OPENAL_LIBS@

AM_CXXFLAGS          = -DSNOWBALLS_CONFIG="\"${pkgsysconfdir}/\"" @FREETYPE_CFLAGS@


# End of Makefile.am

