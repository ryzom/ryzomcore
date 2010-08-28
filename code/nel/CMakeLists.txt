#-----------------------------------------------------------------------------
#
# NeL
#  Authors: Nevrax and the NeL Community
#  Version: 0.7.0
#
# Notes:
#   * Changing install location: add -DCMAKE_INSTALL_PREFIX:PATH=/my/new/path
#   * Changing specific install location variables:
#       * NL_ETC_PREFIX (default: $CMAKE_INSTALL_PREFIX/etc)
#       * NL_SHARE_PREFIX (default: $CMAKE_INSTALL_PREFIX/share)
#       * NL_BIN_PREFIX (default: $CMAKE_INSTALL_PREFIX/bin)
#       * NL_SBIN_PREFIX  (default: $CMAKE_INSTALL_PREFIX/sbin)
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
  # have absolute paths (e.g. -lpthreads)
  cmake_policy(SET CMP0003 NEW)

  # Works around warnings about escaped quotes in ADD_DEFINITIONS
  # statements
  cmake_policy(SET CMP0005 OLD)
ENDIF(COMMAND cmake_policy)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/nel.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

# Force out of source builds.
CHECK_OUT_OF_SOURCE()

IF(WIN32)
  SET(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/include;${CMAKE_INCLUDE_PATH}")
  # Stupid hack for FindOpenAL.cmake
  SET(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty;${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/lib;${CMAKE_LIBRARY_PATH}")
  IF(NOT DEFINED ENV{QTDIR})
    SET(ENV{QTDIR} "c:/qt/4.5.0")
  ENDIF(NOT DEFINED ENV{QTDIR})
ENDIF(WIN32)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(NeL CXX C)
SET(NL_VERSION_MAJOR 0)
SET(NL_VERSION_MINOR 7)
SET(NL_VERSION_PATCH 0)
SET(NL_VERSION "${NL_VERSION_MAJOR}.${NL_VERSION_MINOR}.${NL_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Redirect output files
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)

#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()
NL_SETUP_PREFIX_PATHS()

#-----------------------------------------------------------------------------
#Platform specifics

IF(WITH_STLPORT)
  FIND_PACKAGE(STLport REQUIRED)
  INCLUDE_DIRECTORIES(${STLPORT_INCLUDE_DIR})
ENDIF(WITH_STLPORT)

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(Jpeg)

NL_SETUP_BUILD()

IF(WITH_STATIC_DRIVERS)
  IF(WIN32)
    ADD_DEFINITIONS(/DNL_STATIC)
  ELSE(WIN32)
    ADD_DEFINITIONS(-DNL_STATIC)
  ENDIF(WIN32)
ENDIF(WITH_STATIC_DRIVERS)

# On Windows we need to find DirectInput for NLMISC.
# This is how we get events.
IF(WIN32)
  FIND_PACKAGE(DirectXSDK)

  # On Win32 we can also build the MAX plugins.
  IF(WITH_MAXPLUGIN)
    FIND_PACKAGE(3dsMaxSDK)
  ENDIF(WITH_MAXPLUGIN)
ENDIF(WIN32)

IF(WITH_3D)
  FIND_PACKAGE(FreeType)

  IF(WITH_DRIVER_OPENGL)
    FIND_PACKAGE(OpenGL)
    IF(NOT WIN32)
      IF(APPLE)
        FIND_LIBRARY(CARBON NAMES Carbon)
        FIND_LIBRARY(COCOA NAMES Cocoa)
      ELSE(APPLE)
        FIND_PACKAGE(X11)
        FIND_PACKAGE(XF86VidMode)
        FIND_PACKAGE(XRandR)
      ENDIF(APPLE)
    ENDIF(NOT WIN32)
  ENDIF(WITH_DRIVER_OPENGL)

  IF(WITH_CEGUI)
    FIND_PACKAGE(CEGUI)
  ENDIF(WITH_CEGUI)

ENDIF(WITH_3D)

IF(WITH_SOUND)
  FIND_PACKAGE(Ogg)
  FIND_PACKAGE(Vorbis)

  IF(WITH_DRIVER_OPENAL)
    FIND_PACKAGE(OpenAL)
  ENDIF(WITH_DRIVER_OPENAL)

  IF(WITH_DRIVER_FMOD)
    FIND_PACKAGE(FMOD)
  ENDIF(WITH_DRIVER_FMOD)
ENDIF(WITH_SOUND)

IF(WITH_GTK)
  FIND_PACKAGE(GTK2)
ENDIF(WITH_GTK)

IF(WITH_QT)
  FIND_PACKAGE(Qt4 REQUIRED)
ENDIF(WITH_QT)

IF(WITH_TESTS)
  FIND_PACKAGE(CppTest)


  ENABLE_TESTING()
  ADD_TEST(nel_unit_test ${EXECUTABLE_OUTPUT_PATH}/nel_unit_test --html)
  IF(BUILD_DASHBOARD)
    INCLUDE(Dart)
    SET(SVNCOMMAND svn)
    SET(SVNSOURCEDIR http://dev.ryzom.com/svn/trunk/nel)
    SET(GENERATELOGS svn2cl)
  ENDIF(BUILD_DASHBOARD)
ENDIF(WITH_TESTS)

IF(UNIX)
  SET(prefix ${CMAKE_INSTALL_PREFIX})
  SET(exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)
  SET(libdir "${CMAKE_INSTALL_PREFIX}/lib")
  SET(includedir "${CMAKE_INSTALL_PREFIX}/include")
  SET(enable_ligo ${WITH_LIGO})
  SET(enable_logic ${WITH_LOGIC})
  SET(enable_georges ${WITH_GEORGES})
  SET(enable_net ${WITH_NET})
  SET(enable_3d ${WITH_3D})
  SET(enable_pacs ${WITH_PACS})
  SET(enable_sound ${WITH_SOUND})
  CONFIGURE_FILE(nel-config.in nel-config)

  INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/nel-config DESTINATION bin)
ENDIF(UNIX)

IF(WIN32)
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake)
  SET(WINSOCK2_LIB ws2_32.lib)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)

NL_SETUP_BUILD_FLAGS()

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/PCHSupport.cmake)

IF(FINAL_VERSION)
  ADD_DEFINITIONS(-DFINAL_VERSION=1)
ENDIF(FINAL_VERSION)

# config.h configuration and use by projects
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(include)

IF(WITH_SAMPLES)
  ADD_SUBDIRECTORY(samples)
ENDIF(WITH_SAMPLES)

IF(WITH_TOOLS)
  IF(WIN32)
    FIND_PACKAGE(S3TC)
  ENDIF(WIN32)
  ADD_SUBDIRECTORY(tools)
ENDIF(WITH_TOOLS)

# To build the documention, you will have to enable it
# and then do the equivalent of "make DoxygenDoc".
IF(BUILD_DOCUMENTATION)
    IF(DOT)
        SET(HAVE_DOT YES)
    ELSE(DOT)
        SET(HAVE_DOT NO)
    ENDIF(DOT)
    # This processes our Doxyfile.in and substitutes paths to generate
    # a final Doxyfile
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/Doxyfile.cmake.in ${CMAKE_BINARY_DIR}/doc/Doxyfile)

    ADD_CUSTOM_TARGET(DoxygenDoc ${DOXYGEN} ${CMAKE_BINARY_DIR}/doc/Doxyfile)
ENDIF(BUILD_DOCUMENTATION)

# packaging information
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NeL MMORPG Framework")
SET(CPACK_PACKAGE_VENDOR "NeL")
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/README)
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/COPYING)
SET(CPACK_PACKAGE_VERSION_MAJOR "${NL_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${NL_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${NL_VERSION_PATCH}")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};NeL;ALL;/")
SET(CPACK_PACKAGE_EXECUTABLES "nel${NL_VERSION}" "nel")

# NSIS Specific Packing Setup
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "NeL")
SET(CPACK_NSIS_MODIFY_PATH "ON")
SET(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/resources/nevraxpill.ico)
SET(CPACK_NSIS_MUI_UNIICON ${CMAKE_SOURCE_DIR}/resources/nevraxpill.ico)
SET(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/resources\\\\nel.bmp)
SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} NeL")
SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\dev.ryzom.com")
SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\dev.ryzom.com\\\\projects\\\\nel\\\\wiki")
SET(CPACK_NSIS_CONTACT "matt.raykowski@gmail.com")

## Source Packages
SET(CPACK_PACKAGE_FILE_NAME "nel-${NL_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "nel-${NL_VERSION}")
IF(WIN32)
  #SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_GENERATOR "NSIS;ZIP")
  SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSE(WIN32)
  SET(CPACK_GENERATOR "TGZ")
  SET(CPACK_SOURCE_GENERATOR "TGZ")
ENDIF(WIN32)
set(CPACK_SOURCE_IGNORE_FILES
	"~$"
	"\\\\.cvsignore$"
	"^${CMAKE_SOURCE_DIR}.*/CVS/"
	"^${CMAKE_SOURCE_DIR}.*/\\\\.svn/"
	"^${CMAKE_SOURCE_DIR}/debian/"
	"^${CMAKE_SOURCE_DIR}/old/")
IF(WIN32)
  IF(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
    IF(WITH_QT)
      INCLUDE(${QT_USE_FILE})
      INSTALL(FILES
        "${QT_LIBRARY_DIR}/QtGuid4.dll"
        "${QT_LIBRARY_DIR}/QtXmld4.dll"
        "${QT_LIBRARY_DIR}/QtCored4.dll"
        DESTINATION bin)
    ENDIF(WITH_QT)
  ELSE(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    IF(WITH_QT)
      INCLUDE(${QT_USE_FILE})
      INSTALL(FILES
        "${QT_LIBRARY_DIR}/QtGui4.dll"
        "${QT_LIBRARY_DIR}/QtXml4.dll"
        "${QT_LIBRARY_DIR}/QtCore4.dll"
        DESTINATION bin)
    ENDIF(WITH_QT)
  ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "Release")

  # Install CEGUI and its dependencies.
  IF(WITH_CEGUI)
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIBase.dll" DESTINATION bin)
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIDevilImageCodec.dll" DESTINATION bin)
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIExpatParser.dll" DESTINATION bin)
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIFalagardWRBase.dll" DESTINATION bin)
    INSTALL(FILES "${CEGUI_LIB_DIR}/Devil.dll" DESTINATION bin)
    INSTALL(FILES "${CEGUI_LIB_DIR}/ILU.dll" DESTINATION bin)
  ENDIF(WITH_CEGUI)

  # Only the tools require MFC.
  IF(WITH_TOOLS)
    SET(CMAKE_INSTALL_MFC_LIBRARIES TRUE)
  ENDIF(WITH_TOOLS)
  INCLUDE(InstallRequiredSystemLibraries)
ENDIF(WIN32)

INCLUDE(CPack)

INCLUDE(CMakePackaging.txt)

## Debian Packages
#INCLUDE(UseDebian)
#IF(DEBIAN_FOUND)
#  ADD_DEBIAN_TARGETS(nel)
#ENDIF(DEBIAN_FOUND)
