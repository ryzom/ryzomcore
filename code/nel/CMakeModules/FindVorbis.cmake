# - Locate Vorbis library
# This module defines
#  VORBIS_LIBRARY, the library to link against
#  VORBIS_FOUND, if false, do not try to link to VORBIS
#  VORBIS_INCLUDE_DIR, where to find headers.

IF(VORBIS_LIBRARY AND VORBIS_INCLUDE_DIR)
  # in cache already
  SET(VORBIS_FIND_QUIETLY TRUE)
ENDIF(VORBIS_LIBRARY AND VORBIS_INCLUDE_DIR)


FIND_PATH(VORBIS_INCLUDE_DIR
  vorbis/vorbisfile.h
  PATHS
  $ENV{VORBIS_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(VORBIS_LIBRARY
  NAMES vorbis libvorbis
  PATHS
  $ENV{VORBIS_DIR}/lib
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

FIND_LIBRARY(VORBISFILE_LIBRARY
  NAMES vorbisfile libvorbisfile
  PATHS
  $ENV{VORBIS_DIR}/lib
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

IF(VORBIS_LIBRARY AND VORBISFILE_LIBRARY AND VORBIS_INCLUDE_DIR)
  SET(VORBIS_FOUND "YES")
  SET(VORBIS_LIBRARIES ${VORBIS_LIBRARY} ${VORBISFILE_LIBRARY})
  IF(NOT VORBIS_FIND_QUIETLY)
    MESSAGE(STATUS "Found Vorbis: ${VORBIS_LIBRARY}")
  ENDIF(NOT VORBIS_FIND_QUIETLY)
ELSE(VORBIS_LIBRARY AND VORBISFILE_LIBRARY AND VORBIS_INCLUDE_DIR)
  IF(NOT VORBIS_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Vorbis!")
  ENDIF(NOT VORBIS_FIND_QUIETLY)
ENDIF(VORBIS_LIBRARY AND VORBISFILE_LIBRARY AND VORBIS_INCLUDE_DIR)
