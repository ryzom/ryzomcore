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
FIND_PACKAGE(Lua50 REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(Libwww)
FIND_PACKAGE(X11)


NL_SETUP_BUILD()

NL_CONFIGURE_CHECKS()

FIND_PACKAGE(NeL COMPONENTS nelmisc nelnet nel3d nelgeorges nelligo nelpacs nelsound nelsnd_lowlevel REQUIRED)

###
# An example of finding NeL 3D and CEGUI Renderer.
###
IF(WITH_3D)
  FIND_PACKAGE(FreeType)
#  FIND_PACKAGE(NeL3D)
  FIND_PACKAGE(Jpeg)
#  
#  IF(WITH_DRIVER_OPENGL)
#    FIND_PACKAGE(OpenGL)
#    FIND_PACKAGE(NeLDriverGL)
#  ENDIF(WITH_DRIVER_OPENGL)
#
#  IF(WITH_CEGUI)
#    FIND_PACKAGE(CEGUI)
#    FIND_PACKAGE(NeLCEGUIRenderer)
#  ENDIF(WITH_CEGUI)
#
#  SET(JPEG_LIB jpeg)
ENDIF(WITH_3D)

###
# An example of finding sound and a driver.
###
#IF(WITH_SOUND)
#  FIND_PACKAGE(NeLSound)
#  IF(WITH_DRIVER_OPENAL)
#    FIND_PACKAGE(OpenAL)
#    FIND_PACKAGE(NeLDriverAL)
#  ENDIF(WITH_DRIVER_OPENAL)
#ENDIF(WITH_SOUND)

IF(WIN32)
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake)
  SET(WINSOCK2_LIB ws2_32.lib)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common/src )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common )

NL_SETUP_BUILD_FLAGS()

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(client)
ADD_SUBDIRECTORY(tools)

