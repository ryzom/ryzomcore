# FindLua with Lua 5.5 support for CMake versions whose own FindLua predates it.
#
# Lua 5.5 turned the version macros in lua.h into numbers (LUA_VERSION_MAJOR_N 5) and CMake's FindLua only
# learned that, and the 5.5 search names, in CMake 4.3; older modules (CMake 3.31 of the VS2008 Wine image,
# CMake 4.2 of Ubuntu 26.04) cannot read the version and reject a Lua 5.5 installation as not found.
#
# This module defers to CMake's own FindLua in every other case: when that one understands Lua 5.5, or when
# neither Lua 5.5 is requested (find_package(Lua 5.5)) nor WITH_LUA55 is on. Only an old CMake asked for Lua
# 5.5 uses the search below, which sets the same result variables (LUA_FOUND, Lua_FOUND, LUA_INCLUDE_DIR,
# LUA_LIBRARIES, LUA_LIBRARY, LUA_VERSION_STRING, LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_PATCH).

set(_nl_lua_stock "${CMAKE_ROOT}/Modules/FindLua.cmake")
set(_nl_lua_stock_knows_55 FALSE)
if(EXISTS "${_nl_lua_stock}")
  file(STRINGS "${_nl_lua_stock}" _nl_lua_stock_n REGEX "LUA_VERSION_MAJOR\\(_N\\)")
  if(_nl_lua_stock_n)
    set(_nl_lua_stock_knows_55 TRUE)
  endif()
  unset(_nl_lua_stock_n)
endif()

set(_nl_lua_want_55 FALSE)
if(WITH_LUA55)
  set(_nl_lua_want_55 TRUE)
elseif(Lua_FIND_VERSION AND NOT Lua_FIND_VERSION VERSION_LESS 5.5)
  set(_nl_lua_want_55 TRUE)
endif()

if(_nl_lua_stock_knows_55 OR NOT _nl_lua_want_55)
  include("${_nl_lua_stock}")
  unset(_nl_lua_stock)
  unset(_nl_lua_stock_knows_55)
  unset(_nl_lua_want_55)
  return()
endif()

find_path(LUA_INCLUDE_DIR lua.h
  HINTS ENV LUA_DIR
  PATH_SUFFIXES include/lua5.5 include/lua55 include/lua-5.5 include/lua include
  PATHS ~/Library/Frameworks /Library/Frameworks /sw /opt/local /opt/csw /opt)
find_library(LUA_LIBRARY
  NAMES lua55 lua5.5 lua-5.5 lua.5.5 lua
  HINTS ENV LUA_DIR
  PATH_SUFFIXES lib
  PATHS ~/Library/Frameworks /Library/Frameworks /sw /opt/local /opt/csw /opt)

unset(LUA_VERSION_STRING)
if(LUA_INCLUDE_DIR AND EXISTS "${LUA_INCLUDE_DIR}/lua.h")
  file(STRINGS "${LUA_INCLUDE_DIR}/lua.h" _nl_lua_version_lines
    REGEX "^#define[ \t]+LUA_VERSION_(MAJOR|MINOR|RELEASE)(_N)?[ \t]+")
  foreach(_nl_part MAJOR MINOR RELEASE)
    set(_nl_value "")
    foreach(_nl_line IN LISTS _nl_lua_version_lines)
      if(_nl_line MATCHES "LUA_VERSION_${_nl_part}_N[ \t]+([0-9]+)")
        set(_nl_value "${CMAKE_MATCH_1}")
      elseif(_nl_value STREQUAL "" AND _nl_line MATCHES "LUA_VERSION_${_nl_part}[ \t]+\"([0-9]+)\"")
        set(_nl_value "${CMAKE_MATCH_1}")
      endif()
    endforeach()
    set(LUA_VERSION_${_nl_part} "${_nl_value}")
  endforeach()
  set(LUA_VERSION_PATCH "${LUA_VERSION_RELEASE}")
  if(NOT LUA_VERSION_MAJOR STREQUAL "" AND NOT LUA_VERSION_MINOR STREQUAL "")
    set(LUA_VERSION_STRING "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}.${LUA_VERSION_PATCH}")
  endif()
  unset(_nl_lua_version_lines)
  unset(_nl_part)
  unset(_nl_value)
  unset(_nl_line)
endif()

# libm on the platforms that have it, as the stock module does
if(LUA_LIBRARY)
  if(UNIX AND NOT APPLE AND NOT BEOS AND NOT HAIKU)
    find_library(LUA_MATH_LIBRARY m)
    mark_as_advanced(LUA_MATH_LIBRARY)
    set(LUA_LIBRARIES "${LUA_LIBRARY};${LUA_MATH_LIBRARY}")
  else()
    set(LUA_LIBRARIES "${LUA_LIBRARY}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Lua
  REQUIRED_VARS LUA_LIBRARIES LUA_INCLUDE_DIR
  VERSION_VAR LUA_VERSION_STRING)
mark_as_advanced(LUA_INCLUDE_DIR LUA_LIBRARY)

unset(_nl_lua_stock)
unset(_nl_lua_stock_knows_55)
unset(_nl_lua_want_55)
