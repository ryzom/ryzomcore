#-----------------------------------------------------------------------------
#
#   Object Viewer Widget
#   Copyright (C) 2010 Adrian Jäkel <aj at elane2k dot com>
#
#-----------------------------------------------------------------------------

# This tells the application(s) where to find the installed data.
#ADD_DEFINITIONS(-DDATA_DIR="\\"${NL_SHARE_ABSOLUTE_PREFIX}/georges_editor_qt/\\"")

ADD_SUBDIRECTORY(src)

#INSTALL(DIRECTORY data/
#        DESTINATION share/georges_editor_qt/data
#        COMPONENT data
#        PATTERN "CVS" EXCLUDE
#        PATTERN ".svn" EXCLUDE
#        PATTERN "Makefile*" EXCLUDE)
