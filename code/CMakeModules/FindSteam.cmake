# - Locate Steam API
# This module defines
#  STEAM_LIBRARY, the library to link against
#  VORBIS_FOUND, if false, do not try to link to VORBIS
#  VORBIS_INCLUDE_DIR, where to find headers.

IF(STEAM_LIBRARY AND STEAM_INCLUDE_DIR)
  # in cache already
  SET(Steam_FIND_QUIETLY TRUE)
ENDIF()


FIND_PATH(STEAM_INCLUDE_DIR
  steam_api.h
  PATH_SUFFIXES steam
  PATHS
  $ENV{STEAM_DIR}/public
)

IF(WIN32)
  IF(TARGET_X64)
    SET(STEAM_LIBNAME steam_api64)
    SET(STEAM_PATHNAME redistributable_bin/win64)
  ELSE()
    SET(STEAM_LIBNAME steam_api)
    SET(STEAM_PATHNAME redistributable_bin)
  ENDIF()
ELSEIF(APPLE)
  # universal binary
  SET(STEAM_LIBNAME steam_api)
  SET(STEAM_PATHNAME redistributable_bin/osx32)
ELSE()
  IF(TARGET_X64)
    SET(STEAM_LIBNAME steam_api)
    SET(STEAM_PATHNAME redistributable_bin/linux64)
  ELSE()
    SET(STEAM_LIBNAME steam_api)
    SET(STEAM_PATHNAME redistributable_bin/linux32)
  ENDIF()
ENDIF()

FIND_LIBRARY(STEAM_LIBRARY
  NAMES ${STEAM_LIBNAME}
  PATHS
  $ENV{STEAM_DIR}/${STEAM_PATHNAME}
)

# Don't need to check STEAM_LIBRARY because we're dynamically loading Steam DLL
IF(STEAM_INCLUDE_DIR)
  SET(STEAM_FOUND ON)
  SET(STEAM_LIBRARIES ${STEAM_LIBRARY})
  SET(STEAM_INCLUDE_DIRS ${STEAM_INCLUDE_DIR})
  IF(NOT Steam_FIND_QUIETLY)
    MESSAGE(STATUS "Found Steam: ${STEAM_INCLUDE_DIR}")
  ENDIF()
ELSE()
  IF(NOT Steam_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Steam!")
  ENDIF()
ENDIF()
