# - Locate NeL SOUND library
# This module defines
#  NELSOUND_LIBRARY, the library to link against
#  NELSOUND_FOUND, if false, do not try to link to NELSOUND
#  NELSOUND_INCLUDE_DIRS, where to find headers.

IF(NELSOUND_LIBRARY AND NELSOUND_INCLUDE_DIRS)
  # in cache already
  SET(NELSOUND_FIND_QUIETLY TRUE)
ENDIF(NELSOUND_LIBRARY AND NELSOUND_INCLUDE_DIRS)

FIND_PATH(NELSOUND_INCLUDE_DIRS
  nel/sound/u_audio_mixer.h
  PATHS
  $ENV{NELSOUND_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NELSOUND_LIBRARY
  NAMES nelsound
  PATHS
  $ENV{NELSOUND_DIR}/lib
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

IF(NELSOUND_LIBRARY AND NELSOUND_INCLUDE_DIRS)
  SET(NELSOUND_FOUND "YES")
  IF(NOT NELSOUND_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL SOUND: ${NELSOUND_LIBRARY}")
  ENDIF(NOT NELSOUND_FIND_QUIETLY)
ELSE(NELSOUND_LIBRARY AND NELSOUND_INCLUDE_DIRS)
  IF(NOT NELSOUND_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL SOUND!")
  ENDIF(NOT NELSOUND_FIND_QUIETLY)
ENDIF(NELSOUND_LIBRARY AND NELSOUND_INCLUDE_DIRS)
