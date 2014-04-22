#
# Find the CppTest includes and library
#
# This module defines
# CPPTEST_INCLUDE_DIR, where to find tiff.h, etc.
# CPPTEST_LIBRARIES, where to find the CppTest libraries.
# CPPTEST_FOUND, If false, do not try to use CppTest.

# also defined, but not for general use are
IF(CPPTEST_LIBRARIES AND CPPTEST_INCLUDE_DIR)
  # in cache already
  SET(CPPTEST_FIND_QUIETLY TRUE)
ENDIF(CPPTEST_LIBRARIES AND CPPTEST_INCLUDE_DIR)

FIND_PATH(CPPTEST_INCLUDE_DIR 
  cpptest.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES cppunit cpptest
)

SET(LIBRARY_NAME_RELEASE cpptest)
SET(LIBRARY_NAME_DEBUG cpptestd)

IF(WITH_STLPORT)
  SET(LIBRARY_NAME_RELEASE cpptest_stlport ${LIBRARY_NAME_RELEASE})
  SET(LIBRARY_NAME_DEBUG cpptest_stlportd ${LIBRARY_NAME_DEBUG})
ENDIF(WITH_STLPORT)

FIND_LIBRARY(CPPTEST_LIBRARY_RELEASE
  ${LIBRARY_NAME_RELEASE}
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

FIND_LIBRARY(CPPTEST_LIBRARY_DEBUG
  ${LIBRARY_NAME_DEBUG}
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

IF(CPPTEST_INCLUDE_DIR)
  IF(CPPTEST_LIBRARY_RELEASE)
    SET(CPPTEST_FOUND TRUE)

    SET(CPPTEST_LIBRARIES "optimized;${CPPTEST_LIBRARY_RELEASE}")
    IF(CPPTEST_LIBRARY_DEBUG)
      SET(CPPTEST_LIBRARIES "${CPPTEST_LIBRARIES};debug;${CPPTEST_LIBRARY_DEBUG}")
    ENDIF(CPPTEST_LIBRARY_DEBUG)
  ENDIF(CPPTEST_LIBRARY_RELEASE)
ENDIF(CPPTEST_INCLUDE_DIR)

IF(CPPTEST_FOUND)
  IF(NOT CPPTEST_FIND_QUIETLY)
    MESSAGE(STATUS "Found CppTest: ${CPPTEST_LIBRARIES}")
  ENDIF(NOT CPPTEST_FIND_QUIETLY)
ELSE(CPPTEST_FOUND)
  IF(NOT CPPTEST_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find CppTest!")
  ENDIF(NOT CPPTEST_FIND_QUIETLY)
ENDIF(CPPTEST_FOUND)

MARK_AS_ADVANCED(CPPTEST_LIBRARY_RELEASE CPPTEST_LIBRARY_DEBUG)
