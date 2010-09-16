#
# Find the LibSquish includes and library
#
# This module defines
# SQUISH_INCLUDE_DIR, where to find squish.h
# SQUISH_LIBRARY, where to find the Squish library.
# SQUISH_FOUND, If false, do not try to use Squish.

# also defined, but not for general use are
IF(SQUISH_LIBRARY AND SQUISH_INCLUDE_DIR)
  # in cache already
  SET(SQUISH_FIND_QUIETLY TRUE)
ENDIF(SQUISH_LIBRARY AND SQUISH_INCLUDE_DIR)

FIND_PATH(SQUISH_INCLUDE_DIR 
  squish.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES cppunit
)

FIND_LIBRARY(SQUISH_LIBRARY_RELEASE 
  squish
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

FIND_LIBRARY(SQUISH_LIBRARY_DEBUG
  squishd
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

IF(SQUISH_LIBRARY_RELEASE)
  SET(STLPORT_FOUND "YES")

  SET(SQUISH_LIBRARY "optimized;${SQUISH_LIBRARY_RELEASE}")
  IF(SQUISH_LIBRARY_DEBUG)
    SET(SQUISH_LIBRARY ";debug;${SQUISH_LIBRARY_DEBUG}")
  ENDIF(SQUISH_LIBRARY_DEBUG)
ENDIF(SQUISH_LIBRARY_RELEASE)

IF(SQUISH_LIBRARY AND SQUISH_INCLUDE_DIR)
  SET(SQUISH_FOUND "YES")
  IF(NOT SQUISH_FIND_QUIETLY)
    MESSAGE(STATUS "Found Squish: ${SQUISH_LIBRARY}")
  ENDIF(NOT SQUISH_FIND_QUIETLY)
ELSE(SQUISH_LIBRARY AND SQUISH_INCLUDE_DIR)
  IF(NOT SQUISH_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Squish!")
  ENDIF(NOT SQUISH_FIND_QUIETLY)
ENDIF(SQUISH_LIBRARY AND SQUISH_INCLUDE_DIR)

