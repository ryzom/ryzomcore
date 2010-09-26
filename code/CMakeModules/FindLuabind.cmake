# - Locate Luabind library
# This module defines
#  LUABIND_LIBRARIES, the libraries to link against
#  LUABIND_FOUND, if false, do not try to link to LUABIND
#  LUABIND_INCLUDE_DIR, where to find headers.

IF(LUABIND_LIBRARIES AND LUABIND_INCLUDE_DIR)
  # in cache already
  SET(LUABIND_FIND_QUIETLY TRUE)
ENDIF(LUABIND_LIBRARIES AND LUABIND_INCLUDE_DIR)

FIND_PATH(LUABIND_INCLUDE_DIR
  luabind/luabind.hpp
  PATHS
  $ENV{LUABIND_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

IF(WITH_STLPORT)
  FIND_LIBRARY(LUABIND_LIBRARY_RELEASE
    NAMES luabind_stlport luabind libluabind
    PATHS
    $ENV{LUABIND_DIR}/lib
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

  FIND_LIBRARY(LUABIND_LIBRARY_DEBUG
    NAMES luabind_stlportd luabind_d libluabind_d libluabindd
    PATHS
    $ENV{LUABIND_DIR}/lib
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
ELSE(WITH_STLPORT)
  FIND_LIBRARY(LUABIND_LIBRARY_RELEASE
    NAMES luabind libluabind
    PATHS
    $ENV{LUABIND_DIR}/lib
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

  FIND_LIBRARY(LUABIND_LIBRARY_DEBUG
    NAMES luabind_d libluabind_d libluabindd
    PATHS
    $ENV{LUABIND_DIR}/lib
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
ENDIF(WITH_STLPORT)

IF(LUABIND_INCLUDE_DIR)
  IF(LUABIND_LIBRARY_RELEASE)
    SET(LUABIND_FOUND TRUE)

	SET(LUABIND_LIBRARIES "optimized;${LUABIND_LIBRARY_RELEASE}")
    IF(LUABIND_LIBRARY_DEBUG)
      SET(LUABIND_LIBRARIES "${LUABIND_LIBRARIES};debug;${LUABIND_LIBRARY_DEBUG}")
    ENDIF(LUABIND_LIBRARY_DEBUG)
  ENDIF(LUABIND_LIBRARY_RELEASE)
ENDIF(LUABIND_INCLUDE_DIR)

IF(LUABIND_FOUND)
  IF(NOT LUABIND_FIND_QUIETLY)
    MESSAGE(STATUS "Found Luabind: ${LUABIND_LIBRARIES}")
  ENDIF(NOT LUABIND_FIND_QUIETLY)
ELSE(LUABIND_FOUND)
  IF(NOT LUABIND_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Luabind!")
  ENDIF(NOT LUABIND_FIND_QUIETLY)
ENDIF(LUABIND_FOUND)

MARK_AS_ADVANCED(LUABIND_LIBRARY_RELEASE LUABIND_LIBRARY_DEBUG)
