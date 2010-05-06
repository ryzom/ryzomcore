# - Locate NeL LIGO library
# This module defines
#  NELLIGO_LIBRARY, the library to link against
#  NELLIGO_FOUND, if false, do not try to link to NELLIGO
#  NELLIGO_INCLUDE_DIRS, where to find headers.

IF(NELLIGO_LIBRARY AND NELLIGO_INCLUDE_DIRS)
  # in cache already
  SET(NELLIGO_FIND_QUIETLY TRUE)
ENDIF(NELLIGO_LIBRARY AND NELLIGO_INCLUDE_DIRS)

FIND_PATH(NELLIGO_INCLUDE_DIRS
  nel/ligo/ligo_config.h
  PATHS
  $ENV{NELLIGO_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NELLIGO_LIBRARY
  NAMES nelligo nelligo_rd
  PATHS
  $ENV{NELLIGO_DIR}/lib
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

IF(NELLIGO_LIBRARY AND NELLIGO_INCLUDE_DIRS)
  SET(NELLIGO_FOUND "YES")
  IF(NOT NELLIGO_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL LIGO: ${NELLIGO_LIBRARY}")
  ENDIF(NOT NELLIGO_FIND_QUIETLY)
ELSE(NELLIGO_LIBRARY AND NELLIGO_INCLUDE_DIRS)
  IF(NOT NELLIGO_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL LIGO!")
  ENDIF(NOT NELLIGO_FIND_QUIETLY)
ENDIF(NELLIGO_LIBRARY AND NELLIGO_INCLUDE_DIRS)
