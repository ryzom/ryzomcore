# - Locate NeL SNDDRV library
# This module defines
#  NELSNDDRV_LIBRARY, the library to link against
#  NELSNDDRV_FOUND, if false, do not try to link to NELSNDDRV

IF(NELSNDDRV_LIBRARY)
  # in cache already
  SET(NELSNDDRV_FIND_QUIETLY TRUE)
ENDIF(NELSNDDRV_LIBRARY)

FIND_LIBRARY(NELSNDDRV_LIBRARY
  NAMES nelsnd_lowlevel nelsnd_lowlevel_r nelsnd_lowlevel_d
  PATHS
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\NeL\\NeL;]/lib
  $ENV{ProgramFiles}/NeL/lib
  $ENV{NELSNDDRV_DIR}/lib
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

IF(NELSNDDRV_LIBRARY)
  SET(NELSNDDRV_FOUND "YES")
  IF(NOT NELSNDDRV_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL Sound Lowlevel: ${NELSNDDRV_LIBRARY}")
  ENDIF(NOT NELSNDDRV_FIND_QUIETLY)
ELSE(NELSNDDRV_LIBRARY)
  IF(NOT NELSNDDRV_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL Sound Lowlevel!")
  ENDIF(NOT NELSNDDRV_FIND_QUIETLY)
ENDIF(NELSNDDRV_LIBRARY)
