# - Locate Luabind library
# This module defines
#  LUABIND_LIBRARIES, the libraries to link against
#  LUABIND_FOUND, if false, do not try to link to LUABIND
#  LUABIND_INCLUDE_DIR, where to find headers.

MACRO(FIND_CORRECT_LUA_VERSION)
  # Check Lua version linked to Luabind under Linux
  IF(LUABIND_LIBRARY_RELEASE MATCHES "\\.so")
    INCLUDE(CheckDepends)

    # check for Lua 5.3
    SET(LUA53_LIBRARIES liblua5.3 liblua-5.3 liblua.so.5.3)

    FOREACH(_LIB ${LUA53_LIBRARIES})
      CHECK_LINKED_LIBRARY(LUABIND_LIBRARY_RELEASE _LIB LUALIB_FOUND)
      IF(LUALIB_FOUND)
        MESSAGE(STATUS "Luabind is using Lua 5.3")
        FIND_PACKAGE(Lua53 REQUIRED)
        BREAK()
      ENDIF()
    ENDFOREACH()

    IF(NOT LUALIB_FOUND)
      # check for Lua 5.2
      SET(LUA52_LIBRARIES liblua5.2 liblua-5.2 liblua.so.5.2)

      FOREACH(_LIB ${LUA52_LIBRARIES})
        CHECK_LINKED_LIBRARY(LUABIND_LIBRARY_RELEASE _LIB LUALIB_FOUND)
        IF(LUALIB_FOUND)
          MESSAGE(STATUS "Luabind is using Lua 5.2")
          FIND_PACKAGE(Lua52 REQUIRED)
          BREAK()
        ENDIF()
      ENDFOREACH()
    ENDIF()

    IF(NOT LUALIB_FOUND)
      # check for Lua 5.1
      SET(LUA51_LIBRARIES liblua5.1 liblua-5.1 liblua.so.5.1)

      FOREACH(_LIB ${LUA51_LIBRARIES})
        CHECK_LINKED_LIBRARY(LUABIND_LIBRARY_RELEASE _LIB LUALIB_FOUND)
        IF(LUALIB_FOUND)
          MESSAGE(STATUS "Luabind is using Lua 5.1")
          FIND_PACKAGE(Lua51 REQUIRED)
          BREAK()
        ENDIF()
      ENDFOREACH()
    ENDIF()

    IF(NOT LUALIB_FOUND)
      # check for Lua 5.0
      SET(LUA50_LIBRARIES liblua5.0 liblua-5.0 liblua.so.5.0)

      FOREACH(_LIB ${LUA50_LIBRARIES})
        CHECK_LINKED_LIBRARY(LUABIND_LIBRARY_RELEASE _LIB LUALIB_FOUND)
        IF(LUALIB_FOUND)
          MESSAGE(STATUS "Luabind is using Lua 5.0")
          FIND_PACKAGE(Lua50 REQUIRED)
          BREAK()
        ENDIF()
      ENDFOREACH()
    ENDIF()

    IF(NOT LUALIB_FOUND)
      MESSAGE(FATAL_ERROR "Can't determine Lua version used by Luabind")
    ENDIF()
  ELSE()
    # TODO: find a way to detect Lua version
    IF(WITH_LUA53)
      FIND_PACKAGE(Lua53 REQUIRED)
    ELSEIF(WITH_LUA52)
      FIND_PACKAGE(Lua52 REQUIRED)
    ELSEIF(WITH_LUA51)
      FIND_PACKAGE(Lua51 REQUIRED)
    ELSE()
      FIND_PACKAGE(Lua50 REQUIRED)
    ENDIF()
  ENDIF()
ENDMACRO()

IF(LUABIND_LIBRARIES AND LUABIND_INCLUDE_DIR)
  # in cache already
  SET(Luabind_FIND_QUIETLY TRUE)
ENDIF()

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

SET(LIBRARY_NAME_RELEASE)
SET(LIBRARY_NAME_DEBUG)

IF(WITH_LUA53)
  IF(WITH_STLPORT)
    LIST(APPEND LIBRARY_NAME_RELEASE luabind_stlport_lua53)
    LIST(APPEND LIBRARY_NAME_DEBUG luabind_stlport_lua53d)
  ENDIF()

  LIST(APPEND LIBRARY_NAME_RELEASE luabind_lua53)
  LIST(APPEND LIBRARY_NAME_DEBUG luabind_lua53d)
ENDIF()

IF(WITH_LUA52)
  IF(WITH_STLPORT)
    LIST(APPEND LIBRARY_NAME_RELEASE luabind_stlport_lua52)
    LIST(APPEND LIBRARY_NAME_DEBUG luabind_stlport_lua52d)
  ENDIF()

  LIST(APPEND LIBRARY_NAME_RELEASE luabind_lua52)
  LIST(APPEND LIBRARY_NAME_DEBUG luabind_lua52d)
ENDIF()

IF(WITH_LUA51)
  IF(WITH_STLPORT)
    LIST(APPEND LIBRARY_NAME_RELEASE luabind_stlport_lua51)
    LIST(APPEND LIBRARY_NAME_DEBUG luabind_stlport_lua51d)
  ENDIF()

  LIST(APPEND LIBRARY_NAME_RELEASE luabind_lua51)
  LIST(APPEND LIBRARY_NAME_DEBUG luabind_lua51d)
ENDIF()

IF(WITH_LUA50)
  IF(WITH_STLPORT)
    LIST(APPEND LIBRARY_NAME_RELEASE luabind_stlport_lua50)
    LIST(APPEND LIBRARY_NAME_DEBUG luabind_stlport_lua50d)
  ENDIF()

  LIST(APPEND LIBRARY_NAME_RELEASE luabind_lua50)
  LIST(APPEND LIBRARY_NAME_DEBUG luabind_lua50d)
ENDIF()

IF(WITH_STLPORT)
  LIST(APPEND LIBRARY_NAME_RELEASE luabind_stlport)
  LIST(APPEND LIBRARY_NAME_DEBUG luabind_stlportd)
ENDIF(WITH_STLPORT)

# generic libraries names
LIST(APPEND LIBRARY_NAME_RELEASE luabind libluabind)
LIST(APPEND LIBRARY_NAME_DEBUG luabind_d luabindd libluabind_d libluabindd)

FIND_LIBRARY(LUABIND_LIBRARY_RELEASE
  NAMES ${LIBRARY_NAME_RELEASE}
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
  NAMES ${LIBRARY_NAME_DEBUG}
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

FIND_PACKAGE(Boost REQUIRED)

IF(LUABIND_INCLUDE_DIR AND Boost_INCLUDE_DIR)
  IF(LUABIND_LIBRARY_RELEASE AND LUABIND_LIBRARY_DEBUG)
    # Case where both Release and Debug versions are provided
    SET(LUABIND_FOUND TRUE)
    SET(LUABIND_LIBRARIES optimized ${LUABIND_LIBRARY_RELEASE} debug ${LUABIND_LIBRARY_DEBUG})
  ELSEIF(LUABIND_LIBRARY_RELEASE)
    # Normal case
    SET(LUABIND_FOUND TRUE)
    SET(LUABIND_LIBRARIES ${LUABIND_LIBRARY_RELEASE})
  ELSEIF(LUABIND_LIBRARY_DEBUG)
    # Case where Luabind is compiled from sources (debug version is compiled by default)
    SET(LUABIND_FOUND TRUE)
    SET(LUABIND_LIBRARIES ${LUABIND_LIBRARY_DEBUG})
  ENDIF(LUABIND_LIBRARY_RELEASE AND LUABIND_LIBRARY_DEBUG)
ENDIF()

IF(LUABIND_FOUND)
  SET(LUABIND_INCLUDE_DIR ${LUABIND_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
  # Check if luabind/version.hpp exists
  FIND_FILE(LUABIND_VERSION_FILE luabind/version.hpp PATHS ${LUABIND_INCLUDE_DIR})
  IF(LUABIND_VERSION_FILE)
    SET(LUABIND_DEFINITIONS "-DHAVE_LUABIND_VERSION")
  ENDIF()

  FIND_CORRECT_LUA_VERSION()

  IF(NOT Luabind_FIND_QUIETLY)
    MESSAGE(STATUS "Found Luabind: ${LUABIND_LIBRARIES}")
  ENDIF(NOT Luabind_FIND_QUIETLY)
ELSE()
  IF(NOT Luabind_FIND_QUIETLY)
    MESSAGE(STATUS "Warning: Unable to find Luabind!")
  ENDIF()
ENDIF(LUABIND_FOUND)

MARK_AS_ADVANCED(LUABIND_LIBRARY_RELEASE LUABIND_LIBRARY_DEBUG Boost_LIB_DIAGNOSTIC_DEFINITIONS)
