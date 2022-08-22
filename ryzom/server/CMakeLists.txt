#-----------------------------------------------------------------------------
#
# Ryzom Core
#  Authors: Nevrax and the Ryzom Core Community
#  Version: 1.0.0
#
# Notes:
#   * Changing install location: add -DCMAKE_INSTALL_PREFIX:PATH=/my/new/path
#   * Changing specific install location variables:
#       * NL_ETC_PREFIX (default: $CMAKE_INSTALL_PREFIX/etc)
#       * NL_SHARE_PREFIX (default: $CMAKE_INSTALL_PREFIX/share)
#       * NL_BIN_PREFIX (default: $CMAKE_INSTALL_PREFIX/bin)
#       * NL_SBIN_PREFIX  (default: $CMAKE_INSTALL_PREFIX/sbin)
#       * NL_LIB_PREFIX  (default: $CMAKE_INSTALL_PREFIX/lib)
#       * NL_DRIVER_PREFIX  (default: $CMAKE_INSTALL_PREFIX/lib (windows) or $CMAKE_INSTALL_PREFIX/lib/nel)
#   * Enable building of documentation: add -DBUILD_DOCUMENTATION:BOOL=ON - new make target: DoxygenDoc
#   * Updating version: update header (above) but also update NL_VERSION below.
#   * To build binary archive, use the 'package' target.
#     To build source archive, use the 'package_source' target.

#-----------------------------------------------------------------------------
# Load some macros.
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
#-----------------------------------------------------------------------------
# Set CMake 2.6 Policies.
IF(COMMAND cmake_policy)
  # Works around warnings libraries linked against that don't
  # have absolute paths (e.g. -lpthread)
  cmake_policy(SET CMP0003 NEW)
ENDIF()

INCLUDE(nel)
INCLUDE(ConfigureChecks)
INCLUDE(CheckDepends)
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

# Force out of source builds.
CHECK_OUT_OF_SOURCE()

# To be able to specify a different deployment target on Mac OS X :
# export MACOSX_DEPLOYMENT_TARGET=10.6

IF(CMAKE_VERSION VERSION_GREATER "2.8.10")
  STRING(TIMESTAMP CURRENT_YEAR "%Y")
ELSE()
  SET(CURRENT_YEAR "2016")
ENDIF()

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(RyzomCore CXX C)
SET(NL_VERSION_MAJOR 1)
SET(NL_VERSION_MINOR 0)
SET(NL_VERSION_PATCH 0)
SET(YEAR "2004-${CURRENT_YEAR}")
SET(AUTHOR "Winch Gate and The Ryzom Core Community")

SET(RYZOM_VERSION_MAJOR 3)
SET(RYZOM_VERSION_MINOR 3)
SET(RYZOM_VERSION_PATCH 0)

#-----------------------------------------------------------------------------
# Redirect output files
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# DLL should be in the same directory as EXE under Windows
IF(WIN32)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
ELSE()
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
ENDIF()

#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()
NL_SETUP_NEL_DEFAULT_OPTIONS()
NL_SETUP_RYZOM_DEFAULT_OPTIONS()

NL_SETUP_BUILD()
NL_SETUP_BUILD_FLAGS()

NL_SETUP_PREFIX_PATHS()
RYZOM_SETUP_PREFIX_PATHS()

#-----------------------------------------------------------------------------
#Platform specifics

SETUP_EXTERNAL()

NL_CONFIGURE_CHECKS()

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/PCHSupport.cmake)

IF(FINAL_VERSION)
  ADD_DEFINITIONS(-DFINAL_VERSION=1)
ENDIF()

IF(WITH_SSE2)
  ADD_DEFINITIONS(-DNL_HAS_SSE2)
  IF(WITH_SSE3)
    ADD_DEFINITIONS(-DNL_HAS_SSE3)
  ENDIF()
ENDIF()

#  FIND_PACKAGE(NeL COMPONENTS 3d misc pacs sound nimp REQUIRED)
FIND_PACKAGE(NeL REQUIRED)
INCLUDE_DIRECTORIES(${NEL_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
#Platform specifics
FIND_PACKAGE(RyzomGameShare REQUIRED)

INCLUDE_DIRECTORIES(${RYZOM_GAMESHARE_INCLUDE_DIRS})

SET(RZ_SERVER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Need servershare for build packed collision tool
# Need aishare for build wmap tool
FIND_PACKAGE(MySQL REQUIRED)
FIND_PACKAGE(Mongo)

IF(MONGO_FOUND)
  SET(MONGO_DEFINITIONS -DHAVE_MONGO)
ENDIF()

ADD_SUBDIRECTORY(src)
