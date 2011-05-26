# Look for a directory containing STLport.
#
# The following values are defined
# STLPORT_INCLUDE_DIR - where to find vector, etc.
# STLPORT_LIBRARIES   - link against these to use STLport
# STLPORT_FOUND       - True if the STLport is available.

# also defined, but not for general use are
IF(STLPORT_LIBRARIES AND STLPORT_INCLUDE_DIR)
  # in cache already
  SET(STLPORT_FIND_QUIETLY TRUE)
ENDIF(STLPORT_LIBRARIES AND STLPORT_INCLUDE_DIR)

FIND_PATH(STLPORT_INCLUDE_DIR 
  iostream
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES stlport
)

FIND_LIBRARY(STLPORT_LIBRARY_DEBUG
  NAMES
  stlport_cygwin_debug
  stlport_cygwin_stldebug
  stlport_gcc_debug
  stlport_gcc_stldebug
  stlportstld_x
  stlportstld_x.5.2
  stlportd
  stlportd_statix
  stlportd_static
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

FIND_LIBRARY(STLPORT_LIBRARY_RELEASE
  NAMES
  stlport_cygwin
  stlport_gcc
  stlport
  stlport_x
  stlport_x.5.2
  stlport_statix
  stlport_static
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

IF(STLPORT_INCLUDE_DIR)
  IF(STLPORT_LIBRARY_RELEASE)
    SET(STLPORT_FOUND TRUE)

    SET(STLPORT_LIBRARIES ${STLPORT_LIBRARY_RELEASE})
    IF(STLPORT_LIBRARY_DEBUG)
      SET(STLPORT_LIBRARIES optimized ${STLPORT_LIBRARIES} debug ${STLPORT_LIBRARY_DEBUG})
    ENDIF(STLPORT_LIBRARY_DEBUG)
  ENDIF(STLPORT_LIBRARY_RELEASE)
ENDIF(STLPORT_INCLUDE_DIR)

IF(STLPORT_FOUND)
  IF(NOT STLPORT_FIND_QUIETLY)
    MESSAGE(STATUS "Found STLport: ${STLPORT_LIBRARIES}")
  ENDIF(NOT STLPORT_FIND_QUIETLY)
ELSE(STLPORT_FOUND)
  IF(NOT STLPORT_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find STLport!")
  ENDIF(NOT STLPORT_FIND_QUIETLY)
ENDIF(STLPORT_FOUND)

MARK_AS_ADVANCED(STLPORT_LIBRARY_RELEASE STLPORT_LIBRARY_DEBUG)
