# - Locate CEGUI library
# This module defines
#  CEGUI_LIBRARY, the library to link against
#  CEGUI_FOUND, if false, do not try to link to CEGUI
#  CEGUI_INCLUDE_DIRS, where to find headers.

IF(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
  # in cache already
  SET(CEGUI_FIND_QUIETLY TRUE)
ENDIF(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)


FIND_PATH(CEGUI_INCLUDE_DIRS
  CEGUI
  PATHS
  $ENV{CEGUI_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES cegui CEGUI
)

FIND_LIBRARY(CEGUI_LIBRARY
  NAMES CEGUIBase
  PATHS
  $ENV{CEGUI_DIR}/lib
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
)

GET_FILENAME_COMPONENT(CEGUI_LIB_DIR ${CEGUI_LIBRARY} PATH CACHE)

IF(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
  SET(CEGUI_FOUND "YES")
  SET(CEGUI_INCLUDE_DIRS "${CEGUI_INCLUDE_DIRS}/CEGUI")
  IF(NOT CEGUI_FIND_QUIETLY)
    MESSAGE(STATUS "Found CEGUI: ${CEGUI_LIBRARY}")
  ENDIF(NOT CEGUI_FIND_QUIETLY)
ELSE(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
  IF(NOT CEGUI_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find CEGUI!")
  ENDIF(NOT CEGUI_FIND_QUIETLY)
ENDIF(CEGUI_LIBRARY AND CEGUI_INCLUDE_DIRS)
