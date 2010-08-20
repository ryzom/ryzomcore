# - Locate Luabind library
# This module defines
#  LUABIND_LIBRARY, the library to link against
#  LUABIND_FOUND, if false, do not try to link to LUABIND
#  LUABIND_INCLUDE_DIR, where to find headers.


IF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
  # in cache already
  SET(LUABIND_FIND_QUIETLY TRUE)
ENDIF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)




FIND_PATH(LUABIND_INCLUDE_DIR
  luabind.hpp
  PATHS
  $ENV{LUABIND_DIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  PATH_SUFFIXES luabind
)


FIND_LIBRARY(LUABIND_LIBRARY
  NAMES luabind libluabind luabind_d libluabind_d libluabindd
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


IF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
  SET(LUABIND_FOUND "YES")
  IF(NOT LUABIND_FIND_QUIETLY)
    MESSAGE(STATUS "Found Luabind: ${LUABIND_LIBRARY}")
  ENDIF(NOT LUABIND_FIND_QUIETLY)
ELSE(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
  IF(NOT LUABIND_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Luabind!")
  ENDIF(NOT LUABIND_FIND_QUIETLY)
ENDIF(LUABIND_LIBRARY AND LUABIND_INCLUDE_DIR)
