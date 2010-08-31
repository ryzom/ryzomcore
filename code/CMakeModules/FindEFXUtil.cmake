# - Locate EFX-Util library
# This module defines
#  EFXUTIL_LIBRARY, the library to link against
#  EFXUTIL_FOUND, if false, do not try to link to EFX-Util
#  EFXUTIL_INCLUDE_DIR, where to find headers.

IF(EFXUTIL_LIBRARY AND EFXUTIL_INCLUDE_DIR)
  # in cache already
  SET(EFXUTIL_FIND_QUIETLY TRUE)
ENDIF(EFXUTIL_LIBRARY AND EFXUTIL_INCLUDE_DIR)


FIND_PATH(EFXUTIL_INCLUDE_DIR
  EFX-Util.h
  PATHS
  $ENV{EFXUTIL_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES AL
)

FIND_LIBRARY(EFXUTIL_LIBRARY
  NAMES EFX-Util efxutil libefxutil
  PATHS
  $ENV{EFXUTIL_DIR}/lib
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

IF(EFXUTIL_LIBRARY AND EFXUTIL_INCLUDE_DIR)
  SET(EFXUTIL_FOUND "YES")
  IF(NOT EFXUTIL_FIND_QUIETLY)
    MESSAGE(STATUS "Found EFX-Util: ${EFXUTIL_LIBRARY}")
  ENDIF(NOT EFXUTIL_FIND_QUIETLY)
ELSE(EFXUTIL_LIBRARY AND EFXUTIL_INCLUDE_DIR)
  IF(NOT EFXUTIL_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find EFX-Util!")
  ENDIF(NOT EFXUTIL_FIND_QUIETLY)
ENDIF(EFXUTIL_LIBRARY AND EFXUTIL_INCLUDE_DIR)
