# - Locate Ogg library
# This module defines
#  OGG_LIBRARY, the library to link against
#  OGG_FOUND, if false, do not try to link to OGG
#  OGG_INCLUDE_DIR, where to find headers.

IF(OGG_LIBRARY AND OGG_INCLUDE_DIR)
  # in cache already
  SET(OGG_FIND_QUIETLY TRUE)
ENDIF(OGG_LIBRARY AND OGG_INCLUDE_DIR)


FIND_PATH(OGG_INCLUDE_DIR
  ogg/ogg.h
  PATHS
  $ENV{OGG_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(OGG_LIBRARY
  NAMES ogg libogg
  PATHS
  $ENV{OGG_DIR}/lib
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

IF(OGG_LIBRARY AND OGG_INCLUDE_DIR)
  SET(OGG_FOUND "YES")
  IF(NOT OGG_FIND_QUIETLY)
    MESSAGE(STATUS "Found Ogg: ${OGG_LIBRARY}")
  ENDIF(NOT OGG_FIND_QUIETLY)
ELSE(OGG_LIBRARY AND OGG_INCLUDE_DIR)
  IF(NOT OGG_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Ogg!")
  ENDIF(NOT OGG_FIND_QUIETLY)
ENDIF(OGG_LIBRARY AND OGG_INCLUDE_DIR)
