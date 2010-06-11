# - Locate Jpeg library
# This module defines
#  XF86VidMode_LIBRARY, the library to link against
#  XF86VidMode_FOUND, if false, do not try to link to XF86VidMode
#  XF86VidMode_INCLUDE_DIR, where to find headers.

IF(XF86VidMode_LIBRARY AND XF86VidMode_INCLUDE_DIR)
  # in cache already
  SET(XF86VidMode_FIND_QUIETLY TRUE)
ENDIF(XF86VidMode_LIBRARY AND XF86VidMode_INCLUDE_DIR)


FIND_PATH(XF86VidMode_INCLUDE_DIR
  xf86vmode.h
  PATHS
  $ENV{XF86VidMode_DIR}/include
  /usr/include/X11/
  /usr/X11R6/include/
  PATH_SUFFIXES extensions 
)

FIND_LIBRARY(XF86VidMode_LIBRARY
  Xxf86vm 
  PATHS
  $ENV{XF86VidMode_DIR}/lib
  /usr/X11R6/lib
  /usr/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
)

IF(XF86VidMode_LIBRARY AND XF86VidMode_INCLUDE_DIR)
  SET(XF86VidMode_FOUND "YES")
  SET(XF86VidMode_DEFINITIONS -DXF86VIDMODE)
  IF(NOT XF86VidMode_FIND_QUIETLY)
    MESSAGE(STATUS "Found XF86VidMode: ${XF86VidMode_LIBRARY}")
  ENDIF(NOT XF86VidMode_FIND_QUIETLY)
ELSE(XF86VidMode_LIBRARY AND XF86VidMode_INCLUDE_DIR)
  IF(NOT XF86VidMode_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find XF86VidMode!")
  ENDIF(NOT XF86VidMode_FIND_QUIETLY)
ENDIF(XF86VidMode_LIBRARY AND XF86VidMode_INCLUDE_DIR)

