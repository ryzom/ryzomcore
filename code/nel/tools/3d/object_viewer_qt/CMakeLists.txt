#-----------------------------------------------------------------------------
#
#   Object Viewer Qt
#   Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>
#
#-----------------------------------------------------------------------------

# This tells the application(s) where to find the installed data.
IF(WIN32)
  SET(OVQT_PLUGIN_DIR "plugins")
ELSEIF(APPLE)
  # TODO: under Mac OS X, don't install but copy files in application package
  SET(OVQT_PLUGIN_DIR "plugins")
ELSE(WIN32)
  SET(OVQT_PLUGIN_DIR ${NL_SHARE_PREFIX}/object_viewer_qt/plugins)
  ADD_DEFINITIONS(-DDATA_DIR="\\"${NL_SHARE_ABSOLUTE_PREFIX}/object_viewer_qt/\\"")
ENDIF(WIN32)

IF(UNIX AND WITH_STATIC)
	MESSAGE(FATAL_ERROR "OVQT does not work with static NeL builds on Unix atm.")
ENDIF()

ADD_SUBDIRECTORY(src)

INSTALL(DIRECTORY data/
        DESTINATION ${NL_SHARE_PREFIX}/object_viewer_qt/data
        COMPONENT data
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)

