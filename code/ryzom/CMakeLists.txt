#-----------------------------------------------------------------------------
#
# Ryzom 
#    Author: Spiderweb
#
# Notes:
#   * Changing install location: add -DCMAKE_INSTALL_PREFIX:PATH=/my/new/path
#   * Changing NeL prefix: add -DNEL_PREFIX_PATH:PATH=/my/nel/install
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Load some macros.
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
INCLUDE(CMakeModules/nel.cmake)
INCLUDE(CMakeModules/ConfigureChecks.cmake)

#-----------------------------------------------------------------------------
# Force out of source builds.
CHECK_OUT_OF_SOURCE()

IF(WIN32)
  SET(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/include;${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/lib;${CMAKE_LIBRARY_PATH}")
ENDIF(WIN32)

IF(NOT NL_USING_MASTER_PROJECT)
  IF(DEFINED NEL_PREFIX_PATH)
    SET(CMAKE_INCLUDE_PATH "${NEL_PREFIX_PATH}/include;${CMAKE_INCLUDE_PATH}")
    SET(CMAKE_LIBRARY_PATH "${NEL_PREFIX_PATH}/lib;${CMAKE_LIBRARY_PATH}")
  ELSE(DEFINED NEL_PREFIX_PATH)
    SET(NEL_PREFIX_PATH "" CACHE FILEPATH "NeL Installation Location")
  ENDIF(DEFINED NEL_PREFIX_PATH)
ELSE(NOT NL_USING_MASTER_PROJECT)
    SET(CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/nel/include;${CMAKE_INCLUDE_PATH}")
    SET(CMAKE_LIBRARY_PATH "${CMAKE_BINARY_DIR}/lib;${CMAKE_LIBRARY_PATH}")
ENDIF(NOT NL_USING_MASTER_PROJECT)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(Ryzom CXX C)
SET(NL_VERSION_MAJOR 0)
SET(NL_VERSION_MINOR 6)
SET(NL_VERSION_PATCH 0)
SET(NL_VERSION "${NL_VERSION_MAJOR}.${NL_VERSION_MINOR}.${NL_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Redirect output files
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)

#-----------------------------------------------------------------------------
# Set CMake 2.6 Policies.
IF(COMMAND cmake_policy)
  # Works around warnings libraries linked against that don't
  # have absolute paths (e.g. -lpthreads)
  cmake_policy(SET CMP0003 NEW)

  # Works around warnings about escaped quotes in ADD_DEFINITIONS 
  # statements
  cmake_policy(SET CMP0005 OLD)
ENDIF(COMMAND cmake_policy)

#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()
NL_SETUP_PREFIX_PATHS()

#-----------------------------------------------------------------------------
# Override default options
#SET(WITH_COVERAGE       ON )
SET(WITH_3D	ON )

#-----------------------------------------------------------------------------
#Platform specifics

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
IF(WITH_LUA51)
  FIND_PACKAGE(Lua51 REQUIRED)
ELSE(WITH_LUA51)
  FIND_PACKAGE(Lua50 REQUIRED)
ENDIF(WITH_LUA51)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(Libwww)

IF(NOT WIN32)
  FIND_PACKAGE(X11)
ENDIF(NOT WIN32)

NL_SETUP_BUILD()

NL_CONFIGURE_CHECKS()

FIND_PACKAGE(NeL COMPONENTS nelmisc nelnet nel3d nelgeorges nelligo nelpacs nelsound nelsnd_lowlevel REQUIRED)


IF(WIN32)
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake)
  SET(WINSOCK2_LIB ws2_32.lib)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common/src )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common )

NL_SETUP_BUILD_FLAGS()

IF(FINAL_VERSION)
  ADD_DEFINITIONS(-DFINAL_VERSION=1)
ENDIF(FINAL_VERSION)

ADD_SUBDIRECTORY(common)

IF(WITH_CLIENT)
  ADD_SUBDIRECTORY(client)
ENDIF(WITH_CLIENT)

IF(WITH_TOOLS)
  ADD_SUBDIRECTORY(tools)
ENDIF(WITH_TOOLS)

IF(WITH_SERVER)
  FIND_PACKAGE(MySQL REQUIRED)
  ADD_SUBDIRECTORY(server)
ENDIF(WITH_SERVER)
