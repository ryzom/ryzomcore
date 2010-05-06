# - Locate NeL NET library
# This module defines
#  NELNET_LIBRARY, the library to link against
#  NELNET_FOUND, if false, do not try to link to NELNET
#  NELNET_INCLUDE_DIRS, where to find headers.

IF(NELNET_LIBRARY AND NELNET_INCLUDE_DIRS)
  # in cache already
  SET(NELNET_FIND_QUIETLY TRUE)
ENDIF(NELNET_LIBRARY AND NELNET_INCLUDE_DIRS)

FIND_PATH(NELNET_INCLUDE_DIRS
  nel/net/service.h
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/include
  $ENV{ProgramFiles}/NeL/include
  $ENV{NELNET_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NELNET_LIBRARY
  NAMES nelnet nelnet_r nelnet_d
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/lib
  $ENV{ProgramFiles}/NeL/lib
  $ENV{NELNET_DIR}/lib
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

IF(NELNET_LIBRARY AND NELNET_INCLUDE_DIRS)
  SET(NELNET_FOUND "YES")
  IF(NOT NELNET_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL NET: ${NELNET_LIBRARY}")
  ENDIF(NOT NELNET_FIND_QUIETLY)
ELSE(NELNET_LIBRARY AND NELNET_INCLUDE_DIRS)
  IF(NOT NELNET_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL NET!")
  ENDIF(NOT NELNET_FIND_QUIETLY)
ENDIF(NELNET_LIBRARY AND NELNET_INCLUDE_DIRS)
