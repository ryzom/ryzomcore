#-----------------------------------------------------------------------------
#
# Ryzom Core
#  Authors: Nevrax and the Ryzom Core Community
#  Version: 0.12.0
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.28)

#-----------------------------------------------------------------------------
# Load some macros.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules")
#-----------------------------------------------------------------------------
# Downloads and install Hunter into HUNTER_ROOT directory.
OPTION(HUNTER_ENABLED "Enable Hunter package manager" OFF)
IF(HUNTER_ENABLED)

  INCLUDE("CMakeModules/HunterGate.cmake")
  HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.26.1.tar.gz"
    SHA1 "e41ac7a18c49b35ebac99ff2b5244317b2638a65"
    FILEPATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules/HunterConfig.cmake"
  )

  # default options for hunterized build
  SET(WITH_STATIC         ON CACHE BOOL "static" FORCE)
  SET(WITH_STATIC_DRIVERS ON CACHE BOOL "static drivers" FORCE)
  SET(WITH_STATIC_LIBXML2 ON CACHE BOOL "static libxml2" FORCE)
  SET(WITH_STATIC_EXTERNAL ON CACHE BOOL "static external" FORCE)
  SET(WITH_EXTERNAL       OFF CACHE BOOL "external libs" FORCE)
 
 # Workaround for PCH compilation issue, from curl intsizeof depencency
  ADD_DEFINITIONS(-D__STDC_LIMIT_MACROS)
  # TODO: Still applicable after switching to CMake's builtin precompiled header support?
ENDIF()


#-----------------------------------------------------------------------------

INCLUDE(nel)
INCLUDE(ConfigureChecks)
INCLUDE(CheckDepends)

# Force out of source builds.
CHECK_OUT_OF_SOURCE()

# To be able to specify a different deployment target on Mac OS X :
# export MACOSX_DEPLOYMENT_TARGET=10.6

IF(CMAKE_VERSION VERSION_GREATER "2.8.10")
  STRING(TIMESTAMP CURRENT_YEAR "%Y")
ELSE()
  SET(CURRENT_YEAR "2019")
ENDIF()

PROJECT(RyzomCore VERSION 4.1.0.0 LANGUAGES CXX C)
SET(NL_VERSION_MAJOR ${RyzomCore_VERSION_MAJOR})
SET(NL_VERSION_MINOR ${RyzomCore_VERSION_MINOR})
SET(NL_VERSION_PATCH ${RyzomCore_VERSION_PATCH} CACHE STRING "Patch version")
SET(NL_VERSION_BUILD ${RyzomCore_VERSION_TWEAK} CACHE STRING "Build number")
SET(YEAR "2001-${CURRENT_YEAR}")
SET(AUTHOR "Winch Gate and The Ryzom Core Community")

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

IF(WIN32)
  IF(WITH_MFC)
    FIND_PACKAGE(MFC QUIET)
  ENDIF()
ENDIF()

#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()
NL_SETUP_NEL_DEFAULT_OPTIONS()
NL_SETUP_NELNS_DEFAULT_OPTIONS()
NL_SETUP_RYZOM_DEFAULT_OPTIONS()
NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS()

NL_SETUP_BUILD()
NL_SETUP_BUILD_FLAGS()

NL_SETUP_PREFIX_PATHS()
RYZOM_SETUP_PREFIX_PATHS()

#-----------------------------------------------------------------------------
# Default values for URL's
SET(RYZOM_CLIENT_CREATE_ACCOUNT_URL     "https://core.ryzom.dev/ams/" CACHE STRING "Ryzom Client Create Account URL")
SET(RYZOM_CLIENT_EDIT_ACCOUNT_URL       "https://core.ryzom.dev/ams/" CACHE STRING "Ryzom Client Edit Account URL")
SET(RYZOM_CLIENT_FORGET_PASSWORD_URL    "https://core.ryzom.dev/ams/" CACHE STRING "Ryzom Client Forget Password URL")
SET(RYZOM_CLIENT_PATCH_URL              "https://cdn.ryzom.dev/core4/patch/" CACHE STRING "Ryzom Client Patch URL")
SET(RYZOM_CLIENT_APP_NAME               "core4")

SET(RYZOM_CLIENT_RELEASENOTES_URL       "https://core4.ryzom.dev/releasenotes/index.php" CACHE STRING "Ryzom Client Release Notes URL")
SET(RYZOM_CLIENT_RELEASENOTES_RING_URL  "https://core4.ryzom.dev/releasenotes_ring/index.php" CACHE STRING "Ryzom Client Release Notes URL")

SET(RYZOM_CLIENT_UI_FEATURE_FLAG        "ryzomcore" CACHE STRING "Ryzom Client UI Feature Flag")

#-----------------------------------------------------------------------------
# urls when compiling ryzom live client
IF(WITH_RYZOM_LIVE)
  MESSAGE("Using RYZOM_LIVE urls")
  SET(RYZOM_CLIENT_CREATE_ACCOUNT_URL   "https://account.ryzom.com/signup/from_client.php")
  SET(RYZOM_CLIENT_EDIT_ACCOUNT_URL     "https://account.ryzom.com/payment_profile/index.php")
  SET(RYZOM_CLIENT_FORGET_PASSWORD_URL  "https://account.ryzom.com/payment_profile/lost_secure_password.php")
  SET(RYZOM_CLIENT_PATCH_URL            "http://dl.ryzom.com/patch_live")
  SET(RYZOM_CLIENT_APP_NAME             "ryzom_live")

  SET(RYZOM_CLIENT_RELEASENOTES_URL       "https://app.ryzom.com/releasenotes/index.php")
  SET(RYZOM_CLIENT_RELEASENOTES_RING_URL  "https://app.ryzom.com/releasenotes_ring/index.php")

  SET(RYZOM_CLIENT_UI_FEATURE_FLAG        "ryzomforge")
ENDIF()

#-----------------------------------------------------------------------------
#Platform specifics

SETUP_EXTERNAL()

IF(WIN32)
  SET(WINSOCK2_LIB ws2_32.lib)

  IF(WITH_MFC)
    FIND_PACKAGE(CustomMFC REQUIRED)
  ENDIF()
ENDIF()

IF(HUNTER_ENABLED)
  # This fix is for compiling OpenSSL in Azure Pipeline linux agent where ENV{SYSTEM} == "build"
  IF(DEFINED ENV{SYSTEM} AND UNIX AND NOT APPLE)
    STRING(TOUPPER "$ENV{SYSTEM}" _tmp)
    IF (_tmp STREQUAL "BUILD")
      UNSET(ENV{SYSTEM})
    ENDIF()
    UNSET(_tmp)
  ENDIF()
  ## TODO: hack for freetype hunter package
  SET(ON 1)
  ##
  HUNTER_ADD_PACKAGE(ZLIB)

  FIND_PACKAGE(ZLIB CONFIG REQUIRED)
  # add compatibility with cmake findZLIB module
  if(ZLIB_FOUND AND NOT TARGET ZLIB::ZLIB)
      add_library(ZLIB::ZLIB ALIAS ZLIB::zlib)
  endif()

  HUNTER_ADD_PACKAGE(libxml2)
  FIND_PACKAGE(libxml2 CONFIG REQUIRED)
  # add compatibility with cmake find module
  if(libxml2_FOUND AND NOT TARGET LibXml2::LibXml2)
      add_library(LibXml2::LibXml2 ALIAS libxml2::libxml2)
      IF(WITH_STATIC AND WINSOCK2_LIB)
        # libxml2 with LIBXML_HTTP_ENABLED has some additional dependencies that hunter does not add
            set_property(TARGET libxml2::libxml2
                    APPEND
                    PROPERTY INTERFACE_LINK_LIBRARIES
                    ${WINSOCK2_LIB}
            )
      ENDIF()
  endif()

  # cross compiling doesn't quite work on the hunter openssl package therefor compile it separately and use it
  IF(NOT CMAKE_CROSSCOMPILING)
    HUNTER_ADD_PACKAGE(OpenSSL)
  ENDIF()
  FIND_PACKAGE(OpenSSL REQUIRED COMPONENTS Crypto SSL)

  # for precompiled headers, not needed if using TARGET_PRECOMPILE_HEADERS() from cmake 3.16+
  SET(ZLIB_INCLUDE_DIR ZLIB_ROOT/include)
ELSE()
IF(EMSCRIPTEN)
  # For Emscripten builds, zlib/libpng/libxml2 are not required for the sample demos.
  # Create stub ZLIB::ZLIB imported target so nelmisc's TARGET_LINK_LIBRARIES doesn't
  # error out. No actual zlib is linked; the stub is intentionally empty.
  ADD_LIBRARY(ZLIB::ZLIB INTERFACE IMPORTED)
  SET(ZLIB_FOUND TRUE)
  SET(ZLIB_INCLUDE_DIRS "")
  SET(ZLIB_LIBRARIES "")
  # Do NOT create PNG::PNG - no libpng available, bitmap_png.cpp will be compiled out
  SET(PNG_FOUND FALSE)
  SET(LIBXML2_FOUND FALSE)
  SET(LIBXML2_INCLUDE_DIR "")
  SET(LIBXML2_LIBRARIES "")
ELSE()
  FIND_PACKAGE(ZLIB REQUIRED)
  FIND_PACKAGE(LibXml2 REQUIRED)
  FIND_PACKAGE(PNG REQUIRED)
  FIND_PACKAGE(OpenSSL REQUIRED)
  FIND_PACKAGE(GIF)
  FIND_PACKAGE(JPEG)
ENDIF()

ENDIF() #hunter

IF(WITH_LIBOVR)
  FIND_PACKAGE(LibOVR)
ENDIF()

IF(WITH_LIBVR)
  FIND_PACKAGE(LibVR)
ENDIF()

NL_CONFIGURE_CHECKS()

IF(WITH_STATIC_LIBXML2)
  if(HUNTER_ENABLED AND TARGET libxml2::libxml2)
      set_property(TARGET libxml2::libxml2
              APPEND
              PROPERTY INTERFACE_COMPILE_DEFINITIONS
              LIBXML_STATIC
      )
  elseif(TARGET LibXml2::LibXml2)
      set_property(TARGET LibXml2::LibXml2
              APPEND
              PROPERTY INTERFACE_COMPILE_DEFINITIONS
              LIBXML_STATIC
      )
  endif ()
ENDIF()

IF(NOT EMSCRIPTEN)
  FIND_PACKAGE(Iconv QUIET)
ENDIF()
IF(Iconv_FOUND)
  if(TARGET LibXml2::LibXml2 AND NOT HUNTER_ENABLED)
      set_property(TARGET LibXml2::LibXml2
            APPEND
            PROPERTY INTERFACE_LINK_LIBRARIES
            Iconv::Iconv
      )
  endif ()
ENDIF()

IF(FINAL_VERSION)
  ADD_DEFINITIONS(-DFINAL_VERSION=1)
ENDIF()

IF(WITH_SSE2)
  ADD_DEFINITIONS(-DNL_HAS_SSE2)
  IF(WITH_SSE3)
    ADD_DEFINITIONS(-DNL_HAS_SSE3)
  ENDIF()
ENDIF()

IF(APPLE)
  FIND_LIBRARY(CARBON_FRAMEWORK Carbon)
  FIND_LIBRARY(FOUNDATION_FRAMEWORK Foundation)
  FIND_LIBRARY(SECURITY_FRAMEWORK Security)

  IF(APPLE_CERTIFICATE)
    # Find codesign_allocate

    # Xcode 7.0 and later versions
    SET(CODESIGN_ALLOCATE ${OSX_DEVELOPER_ROOT}/Toolchains/XcodeDefault.xctoolchain/usr/bin/codesign_allocate)

    IF(NOT EXISTS "${CODESIGN_ALLOCATE}")
      # Xcode 6.4 and previous versions
      SET(CODESIGN_ALLOCATE ${CMAKE_OSX_SYSROOT}/usr/bin/codesign_allocate)
    ENDIF()

    IF(NOT EXISTS "${CODESIGN_ALLOCATE}")
      # System path
      SET(CODESIGN_ALLOCATE /usr/bin/codesign_allocate)
    ENDIF()

    IF(NOT EXISTS "${CODESIGN_ALLOCATE}")
      MESSAGE(WARNING "Unable to find codesign_allocate in standard directories")
      SET(CODESIGN_ALLOCATE)
    ENDIF()
  ENDIF()
ENDIF()

INCLUDE(FindHelpers)

#TODO: hunter qt package
IF(WITH_QT5)
  FIND_QT5()
ENDIF()

IF(WITH_QT6)
  FIND_QT6()
ENDIF()

#TODO: hunter assimp package
IF(WITH_ASSIMP)
  FIND_PACKAGE(assimp REQUIRED)
ENDIF()

IF(WITH_MSQUIC)
  FIND_PACKAGE(msquic REQUIRED)
ENDIF()

IF(WITH_NEL_TESTS OR WITH_TESTING)
  ENABLE_TESTING()
  # GoogleTest requires at least C++17 not the whole project depends on google test though
  # set(CMAKE_CXX_STANDARD 17)
  # set(CMAKE_CXX_STANDARD_REQUIRED ON)
  include(FetchContent)
  FetchContent_Declare(
          googletest
          URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
          EXCLUDE_FROM_ALL
  )
  SET(INSTALL_GTEST OFF)
  SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  include(GoogleTest)
  include(cmake/add_test_executable.cmake)
ENDIF()

IF(WITH_NEL)
  IF(HUNTER_ENABLED)
    IF(WITH_GUI)
      HUNTER_ADD_PACKAGE(luabind)
      FIND_PACKAGE(Luabind REQUIRED)
    ENDIF()

    HUNTER_ADD_PACKAGE(CURL)
    FIND_PACKAGE(CURL CONFIG REQUIRED)
  ELSE()
    IF(WITH_GUI)
      FIND_PACKAGE(Luabind REQUIRED)
    ENDIF()

    IF(EMSCRIPTEN)
      # CURL not needed for Emscripten builds
    ELSE()
    FIND_PACKAGE(CURL REQUIRED)
    ENDIF() # !EMSCRIPTEN
  ENDIF() #hunter

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/nel/include)
  ADD_SUBDIRECTORY(nel)

  # allow find_package to find projects from within the mono repo
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/nel/internal")
ELSE()
#  FIND_PACKAGE(NeL COMPONENTS 3d misc pacs sound nimp REQUIRED)
  FIND_PACKAGE(NeL REQUIRED)
ENDIF()

IF(WITH_RYZOM)
  ADD_SUBDIRECTORY(ryzom)
ENDIF()

IF(WITH_NELNS)
  ADD_SUBDIRECTORY(nelns)
ENDIF()

IF(WITH_SNOWBALLS)
  ADD_SUBDIRECTORY(snowballs2)
ENDIF()

IF(WITH_TOOLS)
  ADD_SUBDIRECTORY(tool)
ENDIF()

IF(WITH_STUDIO)
  ADD_SUBDIRECTORY(studio)
ENDIF()

IF(WITH_PERSONAL)
  ADD_SUBDIRECTORY(personal)
ENDIF()

# To build the documention, you will have to enable it
# and then do the equivalent of "make DoxygenDoc".
IF(BUILD_DOCUMENTATION)
    IF(DOT)
        SET(HAVE_DOT YES)
    ELSE()
        SET(HAVE_DOT NO)
    ENDIF()
    # This processes our Doxyfile.in and substitutes paths to generate
    # a final Doxyfile
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/Doxyfile.cmake.in ${CMAKE_BINARY_DIR}/doc/Doxyfile)

    ADD_CUSTOM_TARGET(DoxygenDoc ${DOXYGEN} ${CMAKE_BINARY_DIR}/doc/Doxyfile)
ENDIF()

IF(WITH_NEL_TESTS)
  ENABLE_TESTING()
  IF(BUILD_DASHBOARD)
    INCLUDE(Dart)
    SET(SVNCOMMAND svn)
    SET(SVNSOURCEDIR http://dev.ryzom.com/svn/trunk/nel)
    SET(GENERATELOGS svn2cl)
  ENDIF()
ENDIF()

# packaging information
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ryzom Core MMORPG Framework")
SET(CPACK_PACKAGE_VENDOR "Ryzom Core")
string(TOLOWER ${CMAKE_PROJECT_NAME} PROJECT_NAME_LOWER)
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME_LOWER} CACHE STRING "")
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/README.md)
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/COPYING)
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};NeL;ALL;/")
SET(CPACK_PACKAGE_EXECUTABLES "ryzomcore${NL_VERSION}" "ryzomcore")

# NSIS Specific Packing Setup
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "RyzomCore")
SET(CPACK_PACKAGE_VERSION ${PROJECT_VERSION}) # to include tweak part of version
SET(CPACK_NSIS_MODIFY_PATH "ON")
SET(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/nel/resources/nevraxpill.ico)
SET(CPACK_NSIS_MUI_UNIICON ${CMAKE_SOURCE_DIR}/nel/resources/nevraxpill.ico)
SET(CPACK_PACKAGE_ICON ${CMAKE_SOURCE_DIR}/nel/resources\\\\nel.bmp)
SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} RyzomCore")
SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\dev.ryzom.com")
SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\dev.ryzom.com\\\\projects\\\\nel\\\\wiki")
SET(CPACK_NSIS_CONTACT "matt.raykowski@gmail.com")

## Source Packages
# remove os part of file name
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
IF(WIN32)
  #SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_GENERATOR "NSIS;ZIP")
  SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSE()
  SET(CPACK_GENERATOR "TGZ;STGZ")
  SET(CPACK_SOURCE_GENERATOR "TGZ")
ENDIF()
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
    ENDIF()
  ELSE()
    IF(WITH_QT)
      INCLUDE(${QT_USE_FILE})
      INSTALL(FILES
        "${QT_LIBRARY_DIR}/../bin/QtGui4.dll"
        "${QT_LIBRARY_DIR}/../bin/QtXml4.dll"
        "${QT_LIBRARY_DIR}/../bin/QtCore4.dll"
        DESTINATION ${NL_BIN_PREFIX})
    ENDIF()
  ENDIF()

  # Install CEGUI and its dependencies.
  IF(WITH_NEL_CEGUI)
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIBase.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIDevilImageCodec.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIExpatParser.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/CEGUIFalagardWRBase.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/Devil.dll" DESTINATION ${NL_BIN_PREFIX})
    INSTALL(FILES "${CEGUI_LIB_DIR}/ILU.dll" DESTINATION ${NL_BIN_PREFIX})
  ENDIF()

  # Only the tools require MFC.
  IF(WITH_TOOLS)
    SET(CMAKE_INSTALL_MFC_LIBRARIES TRUE)
  ENDIF()
  #INCLUDE(InstallRequiredSystemLibraries)
ENDIF()

INCLUDE(CPack)

INCLUDE(CMakePackaging.txt)

## Debian Packages
#INCLUDE(UseDebian)
#IF(DEBIAN_FOUND)
#  ADD_DEBIAN_TARGETS(nel)
#ENDIF()
