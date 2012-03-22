# - Locate FreeType library
# This module defines
#  FREETYPE_LIBRARY, the library to link against
#  FREETYPE_FOUND, if false, do not try to link to FREETYPE
#  FREETYPE_INCLUDE_DIRS, where to find headers.

IF(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)
  # in cache already
  SET(FREETYPE_FIND_QUIETLY TRUE)
ENDIF(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)


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
  PATH_SUFFIXES freetype freetype2
)

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
)

# combine both include directories into one variable
IF(FREETYPE_ADDITIONAL_INCLUDE_DIR)
  SET(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS} ${FREETYPE_ADDITIONAL_INCLUDE_DIR})
ENDIF(FREETYPE_ADDITIONAL_INCLUDE_DIR)

FIND_LIBRARY(FREETYPE_LIBRARY
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
)

IF(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)
  SET(FREETYPE_FOUND "YES")
  IF(WITH_STATIC_EXTERNAL AND APPLE)
    FIND_PACKAGE(BZip2)
    IF(BZIP2_FOUND)
      SET(FREETYPE_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS} ${BZIP2_INCLUDE_DIR})
      SET(FREETYPE_LIBRARY ${FREETYPE_LIBRARY} ${BZIP2_LIBRARIES})
    ENDIF(BZIP2_FOUND)
  ENDIF(WITH_STATIC_EXTERNAL AND APPLE)
  IF(NOT FREETYPE_FIND_QUIETLY)
    MESSAGE(STATUS "Found FreeType: ${FREETYPE_LIBRARY}")
  ENDIF(NOT FREETYPE_FIND_QUIETLY)
ELSE(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)
  IF(NOT FREETYPE_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find FreeType!")
  ENDIF(NOT FREETYPE_FIND_QUIETLY)
ENDIF(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)
