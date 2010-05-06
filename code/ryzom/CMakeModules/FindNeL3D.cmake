# - Locate NeL 3D library
# This module defines
#  NEL3D_LIBRARY, the library to link against
#  NEL3D_FOUND, if false, do not try to link to NEL3D
#  NEL3D_INCLUDE_DIRS, where to find headers.

IF(NEL3D_LIBRARY AND NEL3D_INCLUDE_DIRS)
  # in cache already
  SET(NEL3D_FIND_QUIETLY TRUE)
ENDIF(NEL3D_LIBRARY AND NEL3D_INCLUDE_DIRS)

FIND_PATH(NEL3D_INCLUDE_DIRS
  nel/3d/u_driver.h
  PATHS
  $ENV{NEL3D_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
)

FIND_LIBRARY(NEL3D_LIBRARY
  NAMES nel3d
  PATHS
  $ENV{NEL3D_DIR}/lib
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

IF(NEL3D_LIBRARY AND NEL3D_INCLUDE_DIRS)
  SET(NEL3D_FOUND "YES")
  IF(NOT NEL3D_FIND_QUIETLY)
    MESSAGE(STATUS "Found NeL 3D: ${NEL3D_LIBRARY}")
  ENDIF(NOT NEL3D_FIND_QUIETLY)
ELSE(NEL3D_LIBRARY AND NEL3D_INCLUDE_DIRS)
  IF(NOT NEL3D_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find NeL 3D!")
  ENDIF(NOT NEL3D_FIND_QUIETLY)
ENDIF(NEL3D_LIBRARY AND NEL3D_INCLUDE_DIRS)
