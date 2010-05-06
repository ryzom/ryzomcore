#
# $Id: Makefile.am,v 1.1 2002/03/11 14:48:36 puzin Exp $
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS         = client

client_SOURCES       = character_interface.cpp client.cpp \
                       language_interface.cpp \
                       login_interface.cpp move_listener.cpp \
                       pb_message_box.cpp player_view.cpp \
                       shards_list_interface.cpp zone_manager.cpp \
                       zone_search.cpp sight.cpp lens_flare.cpp

client_LDADD         = -lnelnet -lnelmisc -lnel3d @FREETYPE_LIBS@

AM_CXXFLAGS           = @FREETYPE_CFLAGS@


# End of Makefile.am
