# - Locate S3TC library
# This module defines
#  S3TC_LIBRARY, the library to link against
#  S3TC_FOUND, if false, do not try to link to S3TC
#  S3TC_INCLUDE_DIR, where to find headers.

IF(S3TC_LIBRARY AND S3TC_INCLUDE_DIR)
  # in cache already
  SET(S3TC_FIND_QUIETLY TRUE)
ENDIF(S3TC_LIBRARY AND S3TC_INCLUDE_DIR)


FIND_PATH(S3TC_INCLUDE_DIR
  s3_intrf.h
  PATHS
  $ENV{S3TC_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES S3TC
)

FIND_LIBRARY(S3TC_LIBRARY
  NAMES s3tc libs3tc
  PATHS
  $ENV{S3TC_DIR}/lib
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

IF(S3TC_LIBRARY AND S3TC_INCLUDE_DIR)
  SET(S3TC_FOUND "YES")
  IF(NOT S3TC_FIND_QUIETLY)
    MESSAGE(STATUS "Found S3TC: ${S3TC_LIBRARY}")
  ENDIF(NOT S3TC_FIND_QUIETLY)
ELSE(S3TC_LIBRARY AND S3TC_INCLUDE_DIR)
  IF(NOT S3TC_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find S3TC!")
  ENDIF(NOT S3TC_FIND_QUIETLY)
ENDIF(S3TC_LIBRARY AND S3TC_INCLUDE_DIR)
