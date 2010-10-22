#-----------------------------------------------------------------------------
#
#   Object Viewer Qt
#   Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>
#
#-----------------------------------------------------------------------------

# This tells the application(s) where to fidn the installed data.
ADD_DEFINITIONS(-DDATA_DIR="\\"${NL_SHARE_PREFIX}/object_viewer_qt/\\"")

ADD_SUBDIRECTORY(src)

INSTALL(DIRECTORY data/
        DESTINATION share/object_viewer_qt/data
        COMPONENT data
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)

