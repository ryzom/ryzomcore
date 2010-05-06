#
# Find the CppTest includes and library
#
# This module defines
# CPPTEST_INCLUDE_DIR, where to find tiff.h, etc.
# CPPTEST_LIBRARY, where to find the CppTest library.
# CPPTEST_FOUND, If false, do not try to use CppTest.

# also defined, but not for general use are
IF(CPPTEST_LIBRARY AND CPPTEST_INCLUDE_DIR)
  # in cache already
  SET(CPPTEST_FIND_QUIETLY TRUE)
ENDIF(CPPTEST_LIBRARY AND CPPTEST_INCLUDE_DIR)

FIND_PATH(CPPTEST_INCLUDE_DIR 
  cpptest.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES cppunit
)

FIND_LIBRARY(CPPTEST_LIBRARY 
  cpptest
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

IF(CPPTEST_LIBRARY AND CPPTEST_INCLUDE_DIR)
  SET(CPPTEST_FOUND "YES")
  IF(NOT CPPTEST_FIND_QUIETLY)
    MESSAGE(STATUS "Found CppTest: ${CPPTEST_LIBRARY}")
  ENDIF(NOT CPPTEST_FIND_QUIETLY)
ELSE(CPPTEST_LIBRARY AND CPPTEST_INCLUDE_DIR)
  IF(NOT CPPTEST_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find CppTest!")
  ENDIF(NOT CPPTEST_FIND_QUIETLY)
ENDIF(CPPTEST_LIBRARY AND CPPTEST_INCLUDE_DIR)

