#
# Find the W3C libwww includes and library
#
# This module defines
# LIBWWW_INCLUDE_DIR, where to find tiff.h, etc.
# LIBWWW_LIBRARY, where to find the LibWWW library.
# LIBWWW_FOUND, If false, do not try to use LibWWW.

# also defined, but not for general use are
IF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
  # in cache already
  SET(LIBWWW_FIND_QUIETLY TRUE)
ENDIF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)

FIND_PATH(LIBWWW_INCLUDE_DIR 
  WWWInit.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES libwww w3c-libwww
)

FIND_LIBRARY(LIBWWW_LIBRARY 
  wwwapp
  PATHS
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

IF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
  SET(LIBWWW_FOUND "YES")
  IF(NOT LIBWWW_FIND_QUIETLY)
    MESSAGE(STATUS "Found LibWWW: ${LIBWWW_LIBRARY}")
  ENDIF(NOT LIBWWW_FIND_QUIETLY)
ELSE(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)
  IF(NOT LIBWWW_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find LibWWW!")
  ENDIF(NOT LIBWWW_FIND_QUIETLY)
ENDIF(LIBWWW_LIBRARY AND LIBWWW_INCLUDE_DIR)

