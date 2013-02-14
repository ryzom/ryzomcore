#-----------------------------------------------------------------------------
#
#   Georges Editor Qt
#   Copyright (C) 2010 Adrian Jäkel <aj at elane2k dot com>
#
#-----------------------------------------------------------------------------

# This tells the application(s) where to find the installed data.
IF(WIN32)
  SET(GEQT_DATA_DIR ".")
ELSEIF(APPLE)
  # TODO: under Mac OS X, don't install but copy files in application package
  SET(GEQT_DATA_DIR ".")
ELSE(WIN32)
  SET(GEQT_DATA_DIR ${RYZOM_SHARE_PREFIX}/georges_editor_qt/data)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/geqt_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/geqt_config.h)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

  ADD_DEFINITIONS(-DHAVE_GEQT_CONFIG_H)
ENDIF(WIN32)

ADD_SUBDIRECTORY(src)

INSTALL(DIRECTORY data/
        DESTINATION ${GEQT_DATA_DIR}
        COMPONENT data
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)

