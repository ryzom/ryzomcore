#
# Find the CppUnit includes and library
#
# This module defines
# CPPUNIT_INCLUDE_DIR, where to find tiff.h, etc.
# CPPUNIT_LIBRARY, where to find the CppUnit library.
# CPPUNIT_FOUND, If false, do not try to use CppUnit.

# also defined, but not for general use are
IF(CPPUNIT_LIBRARY AND CPPUNIT_INCLUDE_DIR)
  # in cache already
  SET(CPPUNIT_FIND_QUIETLY TRUE)
ENDIF(CPPUNIT_LIBRARY AND CPPUNIT_INCLUDE_DIR)

FIND_PATH(CPPUNIT_INCLUDE_DIR 
  cppunit/TestCase.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES cppunit
)

FIND_LIBRARY(CPPUNIT_LIBRARY 
  cppunit
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

IF(CPPUNIT_LIBRARY AND CPPUNIT_INCLUDE_DIR)
  SET(CPPUNIT_FOUND "YES")
  IF(NOT CPPUNIT_FIND_QUIETLY)
    MESSAGE(STATUS "Found CppUnit: ${CPPUNIT_LIBRARY}")
  ENDIF(NOT CPPUNIT_FIND_QUIETLY)
ELSE(CPPUNIT_LIBRARY AND CPPUNIT_INCLUDE_DIR)
  IF(NOT CPPUNIT_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find CppUnit!")
  ENDIF(NOT CPPUNIT_FIND_QUIETLY)
ENDIF(CPPUNIT_LIBRARY AND CPPUNIT_INCLUDE_DIR)

