# - Locate NeL GEORGES library
# This module defines
#  NELGEORGES_LIBRARY, the library to link against
#  NELGEORGES_FOUND, if false, do not try to link to NELGEORGES
#  NELGEORGES_INCLUDE_DIRS, where to find headers.

IF(NELGEORGES_LIBRARY AND NELGEORGES_INCLUDE_DIRS)
  # in cache already
  SET(NELGEORGES_FIND_QUIETLY TRUE)
ENDIF(NELGEORGES_LIBRARY AND NELGEORGES_INCLUDE_DIRS)

FIND_PATH(NELGEORGES_INCLUDE_DIRS
  nel/georges/u_form_loader.h
  PATHS
  $ENV{NELGEORGES_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NELGEORGES_LIBRARY
  NAMES nelgeorges nelgeorges_rd
  PATHS
  $ENV{NELGEORGES_DIR}/lib
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

IF(NELGEORGES_LIBRARY AND NELGEORGES_INCLUDE_DIRS)
  SET(NELGEORGES_FOUND "YES")
  IF(NOT NELGEORGES_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL GEORGES: ${NELGEORGES_LIBRARY}")
  ENDIF(NOT NELGEORGES_FIND_QUIETLY)
ELSE(NELGEORGES_LIBRARY AND NELGEORGES_INCLUDE_DIRS)
  IF(NOT NELGEORGES_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL GEORGES!")
  ENDIF(NOT NELGEORGES_FIND_QUIETLY)
ENDIF(NELGEORGES_LIBRARY AND NELGEORGES_INCLUDE_DIRS)
