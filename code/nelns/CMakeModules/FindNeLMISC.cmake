# - Locate NeL MISC library
# This module defines
#  NELMISC_LIBRARY, the library to link against
#  NELMISC_FOUND, if false, do not try to link to NELMISC
#  NELMISC_INCLUDE_DIRS, where to find headers.

IF(NELMISC_LIBRARY AND NELMISC_INCLUDE_DIRS)
  # in cache already
  SET(NELMISC_FIND_QUIETLY TRUE)
ENDIF(NELMISC_LIBRARY AND NELMISC_INCLUDE_DIRS)

FIND_PATH(NELMISC_INCLUDE_DIRS
  nel/misc/types_nl.h
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/include
  $ENV{ProgramFiles}/NeL/include
  $ENV{NELMISC_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NELMISC_LIBRARY
  NAMES nelmisc nelmisc_r nelmisc_d
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/lib
  $ENV{ProgramFiles}/NeL/lib
  $ENV{NELMISC_DIR}/lib
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

IF(NELMISC_LIBRARY AND NELMISC_INCLUDE_DIRS)
  SET(NELMISC_FOUND "YES")
  IF(NOT NELMISC_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL MISC: ${NELMISC_LIBRARY}")
  ENDIF(NOT NELMISC_FIND_QUIETLY)
ELSE(NELMISC_LIBRARY AND NELMISC_INCLUDE_DIRS)
  IF(NOT NELMISC_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL MISC!")
  ENDIF(NOT NELMISC_FIND_QUIETLY)
ENDIF(NELMISC_LIBRARY AND NELMISC_INCLUDE_DIRS)
