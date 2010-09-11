#-----------------------------------------------------------------------------
#
# NeLNS
#    Author: Nevrax
#
# Notes:
#   * Changing install location: add -DCMAKE_INSTALL_PREFIX:PATH=/my/new/path
#   * Changing NeL prefix: add -DNEL_PREFIX_PATH:PATH=/my/nel/install
#   * Changing specific install location variables:
#       * NL_ETC_PREFIX (default: $CMAKE_INSTALL_PREFIX/etc)
#       * NL_SHARE_PREFIX (default: $CMAKE_INSTALL_PREFIX/share)
#       * NL_BIN_PREFIX (default: $CMAKE_INSTALL_PREFIX/bin)
#       * NL_SBIN_PREFIX  (default: $CMAKE_INSTALL_PREFIX/sbin)
#   * Updating version: update header (above) but also update NL_VERSION below.
#   * To build binary archive, use the 'package' target.
#     To build source archive, use the 'package_source' target.
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Load some macros.
SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
INCLUDE(CMakeModules/nel.cmake)

#-----------------------------------------------------------------------------
# Force out of source builds.
CHECK_OUT_OF_SOURCE()

IF(WIN32)
  SET(CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/../3rdParty/include;${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/../3rdParty/lib;${CMAKE_INCLUDE_PATH}")
  IF(NOT DEFINED ENV{QTDIR})
    SET(ENV{QTDIR} "c:/qt/4.5.0")
  ENDIF(NOT DEFINED ENV{QTDIR})
ENDIF(WIN32)

IF(DEFINED NEL_PREFIX_PATH)
  SET(CMAKE_INCLUDE_PATH "${NEL_PREFIX_PATH}/include;${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${NEL_PREFIX_PATH}/lib;${CMAKE_LIBRARY_PATH}")
ENDIF(DEFINED NEL_PREFIX_PATH)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(NeLNS CXX C)
SET(NL_VERSION_MAJOR 0)
SET(NL_VERSION_MINOR 7)
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
OPTION(BUILD_SERVICES		"Build NeLNS Services"				ON )
OPTION(BUILD_LOGIN_SYSTEM	"Build NeLNS Login System Tools"	ON )
OPTION(BUILD_RYZOM_LAUNCHER	"Build Ryzom Launcher"				OFF)

#-----------------------------------------------------------------------------
#Platform specifics

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(MySQL)
FIND_PACKAGE(CURL)
FIND_PACKAGE(ZLIB)

IF(WITH_QT)
  FIND_PACKAGE(Qt4 REQUIRED)
ENDIF(WITH_QT)

NL_SETUP_BUILD()

FIND_PACKAGE(NeLMISC)
FIND_PACKAGE(NeLNET)

NL_SETUP_BUILD_FLAGS()

SET(ETCDIR "${NL_ETC_PREFIX}/nelns")
SET(PKGDIR "${NL_SHARE_PREFIX}/nelns")
SET(LOGDIR "${NL_LOG_PREFIX}/nelns")
ADD_DEFINITIONS(-DNELNS_CONFIG="\\"${ETCDIR}\\"" -DNELNS_STATE="\\"${PKGDIR}\\"" -DNELNS_LOGS="\\"${LOGDIR}\\"")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${LOGDIR})")

IF(BUILD_SERVICES)
	ADD_SUBDIRECTORY(admin_executor_service)
	ADD_SUBDIRECTORY(admin_service)
	ADD_SUBDIRECTORY(naming_service)
	ADD_SUBDIRECTORY(login_service)
	ADD_SUBDIRECTORY(welcome_service)
ENDIF(BUILD_SERVICES)

IF(BUILD_LOGIN_SYSTEM)
	ADD_SUBDIRECTORY(login_system)
ENDIF(BUILD_LOGIN_SYSTEM)



#-----------------------------------------------------------------------------
#Packaging
INCLUDE(CMakePackaging.txt)

