# - Locate FreeType library
# This module defines
#  FREETYPE_LIBRARIES, libraries to link against
#  FREETYPE_FOUND, if false, do not try to link to FREETYPE
#  FREETYPE_INCLUDE_DIRS, where to find headers.

IF(FREETYPE_LIBRARIES AND FREETYPE_INCLUDE_DIRS)
  # in cache already
  SET(Freetype_FIND_QUIETLY TRUE)
ENDIF()

FIND_PATH(FREETYPE_INCLUDE_DIRS
  freetype
  PATHS
  $ENV{FREETYPE_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES freetype2
)

IF(NOT FREETYPE_INCLUDE_DIRS)
  SET(FREETYPE_INCLUDE_DIRS "")
ENDIF()

# ft2build.h does not reside in the freetype include dir
FIND_PATH(FREETYPE_ADDITIONAL_INCLUDE_DIR
  ft2build.h
  PATHS
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES freetype2
)

# combine both include directories into one variable
IF(FREETYPE_ADDITIONAL_INCLUDE_DIR)
  SET(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS} ${FREETYPE_ADDITIONAL_INCLUDE_DIR})
ENDIF()

FIND_LIBRARY(FREETYPE_LIBRARY_RELEASE
  NAMES freetype libfreetype freetype219 freetype246
  PATHS
  $ENV{FREETYPE_DIR}/lib
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
  /usr/lib/x86_64-linux-gnu
)

FIND_LIBRARY(FREETYPE_LIBRARY_DEBUG
  NAMES freetyped libfreetyped freetype219d freetype246d
  PATHS
  $ENV{FREETYPE_DIR}/lib
  /usr/local/lib
  /usr/lib
  /usr/local/X11R6/lib
  /usr/X11R6/lib
  /sw/lib
  /opt/local/lib
  /opt/csw/lib
  /opt/lib
  /usr/freeware/lib64
  /usr/lib/x86_64-linux-gnu
)

IF(FREETYPE_INCLUDE_DIRS)
  IF(FREETYPE_LIBRARY_RELEASE AND FREETYPE_LIBRARY_DEBUG)
    # Case where both Release and Debug versions are provided
    SET(FREETYPE_FOUND ON)
    SET(FREETYPE_LIBRARIES optimized ${FREETYPE_LIBRARY_RELEASE} debug ${FREETYPE_LIBRARY_DEBUG})
  ELSEIF(FREETYPE_LIBRARY_RELEASE)
    # Normal case
    SET(FREETYPE_FOUND ON)
    SET(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY_RELEASE})
  ELSEIF(FREETYPE_LIBRARY_DEBUG)
    # Case where Freetype is compiled from sources (debug version is compiled by default)
    SET(FREETYPE_FOUND ON)
    SET(FREETYPE_LIBRARIES ${FREETYPE_LIBRARY_DEBUG})
  ENDIF()
ENDIF()

IF(FREETYPE_FOUND)
  IF(WITH_STATIC_EXTERNAL AND APPLE)
    FIND_PACKAGE(BZip2)
    IF(BZIP2_FOUND)
      SET(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS} ${BZIP2_INCLUDE_DIR})
      SET(FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES} ${BZIP2_LIBRARIES})
    ENDIF()
  ENDIF()
  IF(NOT Freetype_FIND_QUIETLY)
    MESSAGE(STATUS "Found FreeType: ${FREETYPE_LIBRARIES}")
  ENDIF()
ELSE()
  IF(NOT Freetype_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find FreeType!")
  ENDIF()
ENDIF()
