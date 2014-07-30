#-----------------------------------------------------------------------------
#
# NeL
#  Authors: Nevrax and the NeL Community
#  Version: 0.9.0
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

  # Works around warnings about escaped quotes in ADD_DEFINITIONS
  # statements
  cmake_policy(SET CMP0005 OLD)
ENDIF(COMMAND cmake_policy)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/nel.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/ConfigureChecks.cmake)
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/CheckDepends.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)

# Force out of source builds.
CHECK_OUT_OF_SOURCE()

# To be able to specify a different deployment target on Mac OS X :
# export MACOSX_DEPLOYMENT_TARGET=10.6

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(RyzomCore CXX C)
SET(NL_VERSION_MAJOR 0)
SET(NL_VERSION_MINOR 9)
SET(NL_VERSION_PATCH 0)
SET(NL_VERSION "${NL_VERSION_MAJOR}.${NL_VERSION_MINOR}.${NL_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Redirect output files
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# DLL should be in the same directory as EXE under Windows
IF(WIN32)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
ELSE(WIN32)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
ENDIF(WIN32)

IF(WIN32)
  IF(WITH_MFC)
    FIND_PACKAGE(MFC QUIET)
  ENDIF(WITH_MFC)

  IF(NOT DEFINED ENV{QTDIR})
    SET(ENV{QTDIR} "c:/qt/4.6.3")
  ENDIF(NOT DEFINED ENV{QTDIR})
ENDIF(WIN32)

#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()
NL_SETUP_NEL_DEFAULT_OPTIONS()
NL_SETUP_NELNS_DEFAULT_OPTIONS()
NL_SETUP_RYZOM_DEFAULT_OPTIONS()
NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS()

NL_SETUP_PREFIX_PATHS()
RYZOM_SETUP_PREFIX_PATHS()

NL_CONFIGURE_CHECKS()

NL_SETUP_BUILD()
NL_SETUP_BUILD_FLAGS()

#-----------------------------------------------------------------------------
#Platform specifics

SETUP_EXTERNAL()
NL_GEN_REVISION_H()

IF(WIN32)
  SET(WINSOCK2_LIB ws2_32.lib)

  IF(WITH_MFC)
    FIND_PACKAGE(CustomMFC REQUIRED)
  ENDIF(WITH_MFC)
ENDIF(WIN32)

FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(PNG REQUIRED)
FIND_PACKAGE(Jpeg)

IF(WITH_STATIC_LIBXML2)
  SET(LIBXML2_DEFINITIONS ${LIBXML2_DEFINITIONS} -DLIBXML_STATIC)
ENDIF(WITH_STATIC_LIBXML2)

IF(WITH_STATIC)
  # libxml2 could need winsock2 library
  SET(LIBXML2_LIBRARIES ${LIBXML2_LIBRARIES} ${WINSOCK2_LIB})

  # on Mac OS X libxml2 requires iconv and liblzma
  IF(APPLE)
    FIND_PACKAGE(Iconv REQUIRED)
    FIND_PACKAGE(LibLZMA REQUIRED)
    SET(LIBXML2_LIBRARIES ${LIBXML2_LIBRARIES} ${ICONV_LIBRARIES} ${LIBLZMA_LIBRARIES})
    INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
  ENDIF(APPLE)
ENDIF(WITH_STATIC)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/PCHSupport.cmake)

IF(FINAL_VERSION)
  ADD_DEFINITIONS(-DFINAL_VERSION=1)
ENDIF(FINAL_VERSION)

IF(WITH_SSE2)
  ADD_DEFINITIONS(-DNL_HAS_SSE2)
  IF(WITH_SSE3)
    ADD_DEFINITIONS(-DNL_HAS_SSE3)
  ENDIF(WITH_SSE3)
ENDIF(WITH_SSE2)

IF(WITH_QT)
  FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
ENDIF(WITH_QT)

IF(WITH_NEL)
  IF(WITH_NEL_TESTS)
    FIND_PACKAGE(CppTest)
  ENDIF(WITH_NEL_TESTS)

  IF(WITH_GUI)
    FIND_PACKAGE(Libwww REQUIRED)
    FIND_PACKAGE(Luabind REQUIRED)
    FIND_PACKAGE(CURL REQUIRED)

    IF(WIN32 OR CURL_LIBRARIES MATCHES "\\.a")
      SET(CURL_STATIC ON)
    ENDIF(WIN32 OR CURL_LIBRARIES MATCHES "\\.a")

    IF(CURL_STATIC)
      SET(CURL_DEFINITIONS -DCURL_STATICLIB)

      FIND_PACKAGE(OpenSSL QUIET)

      IF(OPENSSL_FOUND)
        SET(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
        SET(CURL_LIBRARIES ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})
      ENDIF(OPENSSL_FOUND)

      # CURL Macports version depends on libidn, libintl and libiconv too
      IF(APPLE)
        FIND_LIBRARY(IDN_LIBRARY idn)
        FIND_LIBRARY(INTL_LIBRARY intl)

        SET(CURL_LIBRARIES ${CURL_LIBRARIES} ${IDN_LIBRARY} ${INTL_LIBRARY})
      ENDIF(APPLE)
    ENDIF(CURL_STATIC)
  ENDIF(WITH_GUI)

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/nel/include ${LIBXML2_INCLUDE_DIR})
  ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})
  ADD_SUBDIRECTORY(nel)
ENDIF(WITH_NEL)

IF(WITH_RYZOM)
  ADD_SUBDIRECTORY(ryzom)
ENDIF(WITH_RYZOM)

IF(WITH_NELNS)
  ADD_SUBDIRECTORY(nelns)
ENDIF(WITH_NELNS)

IF(WITH_SNOWBALLS)
  ADD_SUBDIRECTORY(snowballs2)
ENDIF(WITH_SNOWBALLS)

IF(WITH_TOOLS)
  ADD_SUBDIRECTORY(tool)
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

IF(WITH_NEL_TESTS)
  ENABLE_TESTING()
  ADD_TEST(nel_unit_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/nel_unit_test --html)
  IF(BUILD_DASHBOARD)
    INCLUDE(Dart)
    SET(SVNCOMMAND svn)
    SET(SVNSOURCEDIR http://dev.ryzom.com/svn/trunk/nel)
    SET(GENERATELOGS svn2cl)
  ENDIF(BUILD_DASHBOARD)
ENDIF(WITH_NEL_TESTS)

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
        "${QT_LIBRARY_DIR}/../bin/QtGuid4.dll"
        "${QT_LIBRARY_DIR}/../bin/QtXmld4.dll"
        "${QT_LIBRARY_DIR}/../bin/QtCored4.dll"
        DESTINATION ${NL_BIN_PREFIX})
    ENDIF(WITH_QT)
  ELSE(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    IF(WITH_QT)
      INCLUDE(${QT_USE_FILE})
      INSTALL(FILES
        "${QT_LIBRARY_DIR}/../bin/QtGui4.dll"
        "${QT_LIBRARY_DIR}/../bin/QtXml4.dll"
        "${QT_LIBRARY_DIR}/../bin/QtCore4.dll"
        DESTINATION ${NL_BIN_PREFIX})
    ENDIF(WITH_QT)
  ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "Release")

  # Install CEGUI and its dependencies.
  IF(WITH_NEL_CEGUI)
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIBase.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIDevilImageCodec.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIExpatParser.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIFalagardWRBase.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/Devil.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/ILU.dll" DESTINATION ${NL_BIN_PREFIX})
  ENDIF(WITH_NEL_CEGUI)

  # Only the tools require MFC.
  IF(WITH_TOOLS)
    SET(CMAKE_INSTALL_MFC_LIBRARIES TRUE)
  ENDIF(WITH_TOOLS)
  #INCLUDE(InstallRequiredSystemLibraries)
ENDIF(WIN32)

INCLUDE(CPack)

INCLUDE(CMakePackaging.txt)

## Debian Packages
#INCLUDE(UseDebian)
#IF(DEBIAN_FOUND)
#  ADD_DEBIAN_TARGETS(nel)
#ENDIF(DEBIAN_FOUND)
