# - Locate NeL PACS library
# This module defines
#  NELPACS_LIBRARY, the library to link against
#  NELPACS_FOUND, if false, do not try to link to NELPACS
#  NELPACS_INCLUDE_DIRS, where to find headers.

IF(NELPACS_LIBRARY AND NELPACS_INCLUDE_DIRS)
  # in cache already
  SET(NELPACS_FIND_QUIETLY TRUE)
ENDIF(NELPACS_LIBRARY AND NELPACS_INCLUDE_DIRS)

FIND_PATH(NELPACS_INCLUDE_DIRS
  nel/pacs/u_global_retriever.h
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/include
  $ENV{ProgramFiles}/NeL/include
  $ENV{NELPACS_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NELPACS_LIBRARY
  NAMES nelpacs nelpacs_r nelpacs_d
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/lib
  $ENV{ProgramFiles}/NeL/lib
  $ENV{NELPACS_DIR}/lib
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

IF(NELPACS_LIBRARY AND NELPACS_INCLUDE_DIRS)
  SET(NELPACS_FOUND "YES")
  IF(NOT NELPACS_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL PACS: ${NELPACS_LIBRARY}")
  ENDIF(NOT NELPACS_FIND_QUIETLY)
ELSE(NELPACS_LIBRARY AND NELPACS_INCLUDE_DIRS)
  IF(NOT NELPACS_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL PACS!")
  ENDIF(NOT NELPACS_FIND_QUIETLY)
ENDIF(NELPACS_LIBRARY AND NELPACS_INCLUDE_DIRS)
