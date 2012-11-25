#
# Find the LibSquish includes and library
#
# This module defines
# SQUISH_INCLUDE_DIR, where to find squish.h
# SQUISH_LIBRARIES, where to find the Squish libraries.
# SQUISH_FOUND, If false, do not try to use Squish.

# also defined, but not for general use are
IF(SQUISH_LIBRARIES AND SQUISH_INCLUDE_DIR)
  # in cache already
  SET(SQUISH_FIND_QUIETLY TRUE)
ENDIF(SQUISH_LIBRARIES AND SQUISH_INCLUDE_DIR)

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

IF(SQUISH_INCLUDE_DIR)
  IF(SQUISH_LIBRARY_RELEASE)
    SET(SQUISH_FOUND "YES")

    SET(SQUISH_LIBRARIES "optimized;${SQUISH_LIBRARY_RELEASE}")
    IF(SQUISH_LIBRARY_DEBUG)
      SET(SQUISH_LIBRARIES "${SQUISH_LIBRARIES};debug;${SQUISH_LIBRARY_DEBUG}")
    ENDIF(SQUISH_LIBRARY_DEBUG)
  ENDIF(SQUISH_LIBRARY_RELEASE)
ENDIF(SQUISH_INCLUDE_DIR)

IF(SQUISH_FOUND)
  IF(NOT SQUISH_FIND_QUIETLY)
    MESSAGE(STATUS "Found Squish: ${SQUISH_LIBRARIES}")
  ENDIF(NOT SQUISH_FIND_QUIETLY)
  FILE(STRINGS ${SQUISH_INCLUDE_DIR}/squish.h METRIC REGEX "metric = 0")
  IF(METRIC)
    SET(SQUISH_COMPRESS_HAS_METRIC ON)
    SET(SQUISH_DEFINITIONS -DSQUISH_COMPRESS_HAS_METRIC)
  ENDIF(METRIC)
ELSE(SQUISH_FOUND)
  IF(NOT SQUISH_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Squish!")
  ENDIF(NOT SQUISH_FIND_QUIETLY)
ENDIF(SQUISH_FOUND)

MARK_AS_ADVANCED(SQUISH_LIBRARY_RELEASE SQUISH_LIBRARY_DEBUG)
