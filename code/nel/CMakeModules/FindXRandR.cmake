# - Locate XRandR library
# This module defines
#  XRandR_LIBRARY, the library to link against
#  XRandR_FOUND, if false, do not try to link to XRandR
#  XRandR_INCLUDE_DIR, where to find headers.

IF(XRandR_LIBRARY AND XRandR_INCLUDE_DIR)
  # in cache already
  SET(XRandR_FIND_QUIETLY TRUE)
ENDIF(XRandR_LIBRARY AND XRandR_INCLUDE_DIR)


FIND_PATH(XRandR_INCLUDE_DIR
  Xrandr.h
  PATHS
  $ENV{XRandR_DIR}/include
  /usr/include/X11/
  /usr/X11R6/include/
  PATH_SUFFIXES extensions 
)

FIND_LIBRARY(XRandR_LIBRARY
  Xrandr 
  PATHS
  $ENV{XRandR_DIR}/lib
  /usr/X11R6/lib
  /usr/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
)

IF(XRandR_LIBRARY AND XRandR_INCLUDE_DIR)
  SET(XRandR_FOUND "YES")
  SET(XRandR_DEFINITIONS -DXRANDR)
  IF(NOT XRandR_FIND_QUIETLY)
    MESSAGE(STATUS "Found XRandR: ${XRandR_LIBRARY}")
  ENDIF(NOT XRandR_FIND_QUIETLY)
ELSE(XRandR_LIBRARY AND XRandR_INCLUDE_DIR)
  IF(NOT XRandR_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find XRandR!")
  ENDIF(NOT XRandR_FIND_QUIETLY)
ENDIF(XRandR_LIBRARY AND XRandR_INCLUDE_DIR)

