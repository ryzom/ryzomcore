# - Locate Jpeg library
# This module defines
#  JPEG_LIBRARY, the library to link against
#  JPEG_FOUND, if false, do not try to link to JPEG
#  JPEG_INCLUDE_DIR, where to find headers.

IF(JPEG_LIBRARY AND JPEG_INCLUDE_DIR)
  # in cache already
  SET(JPEG_FIND_QUIETLY TRUE)
ENDIF(JPEG_LIBRARY AND JPEG_INCLUDE_DIR)


FIND_PATH(JPEG_INCLUDE_DIR
  jpeglib.h
  PATHS
  $ENV{JPEG_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES jpeg
)

FIND_LIBRARY(JPEG_LIBRARY
  NAMES jpeg libjpeg
  PATHS
  $ENV{JPEG_DIR}/lib
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

IF(JPEG_LIBRARY AND JPEG_INCLUDE_DIR)
  SET(JPEG_FOUND "YES")
  IF(NOT JPEG_FIND_QUIETLY)
    MESSAGE(STATUS "Found Jpeg: ${JPEG_LIBRARY}")
  ENDIF(NOT JPEG_FIND_QUIETLY)
ELSE(JPEG_LIBRARY AND JPEG_INCLUDE_DIR)
  IF(NOT JPEG_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Jpeg!")
  ENDIF(NOT JPEG_FIND_QUIETLY)
ENDIF(JPEG_LIBRARY AND JPEG_INCLUDE_DIR)
