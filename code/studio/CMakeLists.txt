#-----------------------------------------------------------------------------
#
#   Object Viewer Qt
#   Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>
#
#-----------------------------------------------------------------------------

# This tells the application(s) where to find the installed data.
IF(WIN32)
  SET(OVQT_PLUGIN_SPECS_DIR "plugins")
  SET(OVQT_PLUGIN_DIR "plugins")
  SET(OVQT_DATA_DIR ".")
ELSEIF(APPLE)
  # TODO: under Mac OS X, don't install but copy files in application package
  SET(OVQT_PLUGIN_SPECS_DIR "plugins")
  SET(OVQT_PLUGIN_DIR "plugins")
  SET(OVQT_DATA_DIR ".")
ELSE(WIN32)
  SET(OVQT_PLUGIN_SPECS_DIR ${NL_SHARE_PREFIX}/studio/plugins)
  SET(OVQT_PLUGIN_DIR ${NL_LIB_PREFIX}/studio)
  SET(OVQT_DATA_DIR ${NL_SHARE_PREFIX}/studio/data)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/ovqt_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ovqt_config.h)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

  ADD_DEFINITIONS(-DHAVE_OVQT_CONFIG_H)
ENDIF(WIN32)

IF(UNIX AND WITH_STATIC)
	MESSAGE(FATAL_ERROR "Studio does not work with static NeL builds on Unix atm.")
ENDIF()

ADD_SUBDIRECTORY(src)

INSTALL(DIRECTORY data/
        DESTINATION ${OVQT_DATA_DIR}
        COMPONENT data
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)

