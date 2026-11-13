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
#       * NL_LIB_PREFIX  (default: $CMAKE_INSTALL_PREFIX/$CMAKE_INSTALL_LIBDIR, or lib and lib64 on Windows)
#       * NL_DRIVER_PREFIX  (default: $CMAKE_INSTALL_PREFIX/lib (windows) or $CMAKE_INSTALL_PREFIX/$CMAKE_INSTALL_LIBDIR/nel)
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
    # v0.26.7+ carries the Visual Studio 2026 MSVC_VERSION mapping
    # (0.26.1 dies with "Unexpected MSVC_VERSION: '1951'" on VS2026 runners)
    URL "https://github.com/cpp-pm/hunter/archive/v0.26.10.tar.gz"
    SHA1 "3d904b93ba684f0e671a2d44969a1a127abe3e66"
    FILEPATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules/HunterConfig.cmake"
  )

  # default options for hunterized build
  SET(WITH_STATIC         ON CACHE BOOL "static" FORCE)
  SET(WITH_STATIC_DRIVERS ON CACHE BOOL "static drivers" FORCE)
  SET(WITH_STATIC_LIBXML2 ON CACHE BOOL "static libxml2" FORCE)
  SET(WITH_STATIC_EXTERNAL ON CACHE BOOL "static external" FORCE)
  SET(WITH_EXTERNAL       OFF CACHE BOOL "external libs" FORCE)
 
 # Workaround for PCH compilation issue, from curl intsizeof depencency
  ADD_COMPILE_DEFINITIONS(__STDC_LIMIT_MACROS)
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
SET(RYZOM_CLIENT_CREATE_ACCOUNT_URL     "https://core.ryzom.dev/account/?page=register" CACHE STRING "Ryzom Client Create Account URL")
SET(RYZOM_CLIENT_EDIT_ACCOUNT_URL       "https://core.ryzom.dev/account/?page=settings" CACHE STRING "Ryzom Client Edit Account URL")
SET(RYZOM_CLIENT_FORGET_PASSWORD_URL    "https://core.ryzom.dev/account/" CACHE STRING "Ryzom Client Forget Password URL")
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
# code guarded by RYZOM_FORGE
IF(WITH_RYZOM_FORGE)
  MESSAGE("Building the code guarded by RYZOM_FORGE")
  ADD_DEFINITIONS(-DRYZOM_FORGE)
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
  # For Emscripten builds, zlib and libpng come from Emscripten ports
  # (-sUSE_ZLIB=1 / -sUSE_LIBPNG=1), same pattern as the FreeType port in
  # nel/CMakeLists.txt. The options must be present at both compile time
  # (port headers) and link time (port libraries); INTERFACE options on the
  # imported targets propagate to consumers of nelmisc. libxml2 stays
  # unavailable; XML-dependent code is compiled out.
  ADD_LIBRARY(ZLIB::ZLIB INTERFACE IMPORTED)
  SET_TARGET_PROPERTIES(ZLIB::ZLIB PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_ZLIB=1"
    INTERFACE_LINK_OPTIONS "-sUSE_ZLIB=1")
  SET(ZLIB_FOUND TRUE)
  SET(ZLIB_INCLUDE_DIRS "")
  SET(ZLIB_LIBRARIES "")
  ADD_LIBRARY(PNG::PNG INTERFACE IMPORTED)
  SET_TARGET_PROPERTIES(PNG::PNG PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_ZLIB=1;-sUSE_LIBPNG=1"
    INTERFACE_LINK_OPTIONS "-sUSE_ZLIB=1;-sUSE_LIBPNG=1")
  SET(PNG_FOUND TRUE)
  # libxml2 becomes available when a cross-built static libxml2 is provided
  # (e.g. -DLibXml2_DIR=<prefix>/lib/cmake/libxml2-<version>); without one,
  # XML-dependent code is compiled out as before.
  FIND_PACKAGE(LibXml2 QUIET CONFIG)
  IF(TARGET LibXml2::LibXml2)
    SET(LIBXML2_FOUND TRUE)
  ELSE()
    SET(LIBXML2_FOUND FALSE)
    SET(LIBXML2_INCLUDE_DIR "")
    SET(LIBXML2_LIBRARIES "")
  ENDIF()
  # OpenSSL only when provided (stub or cross-built) via OPENSSL_* variables.
  # Same emscripten find_library-mode-ONLY caveat as CURL below: build the
  # imported targets directly from the -D cache vars. Skipping find_package
  # entirely — when it does run under the emscripten toolchain it can create
  # the OpenSSL::SSL/Crypto targets with an empty or host-side INCLUDE_DIR
  # (find_path is subject to the same ROOT_PATH restriction as find_library),
  # and then our -D INCLUDE_DIR never reaches consumers of nelweb/nelmisc.
  IF(OPENSSL_INCLUDE_DIR AND OPENSSL_SSL_LIBRARY AND NOT TARGET OpenSSL::SSL)
    ADD_LIBRARY(OpenSSL::SSL STATIC IMPORTED)
    SET_TARGET_PROPERTIES(OpenSSL::SSL PROPERTIES
      IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
  ENDIF()
  IF(OPENSSL_INCLUDE_DIR AND OPENSSL_CRYPTO_LIBRARY AND NOT TARGET OpenSSL::Crypto)
    ADD_LIBRARY(OpenSSL::Crypto STATIC IMPORTED)
    SET_TARGET_PROPERTIES(OpenSSL::Crypto PROPERTIES
      IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
  ENDIF()
  # If someone else already published the imported target (find_package that
  # ran earlier via a different path) but with an empty include dir, forcibly
  # append ours so consumers still see the headers.
  IF(TARGET OpenSSL::SSL AND OPENSSL_INCLUDE_DIR)
    SET_PROPERTY(TARGET OpenSSL::SSL APPEND PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
  ENDIF()
  IF(TARGET OpenSSL::Crypto AND OPENSSL_INCLUDE_DIR)
    SET_PROPERTY(TARGET OpenSSL::Crypto APPEND PROPERTY
      INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}")
  ENDIF()
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

# A static libxml2 does not bring the libraries it needs (ICU, lzma, zlib, ...), and CMake's FindLibXml2
# adds none to its target, unlike the CMake packages of libxml2 and vcpkg. LIBXML2_STATIC_DEPENDENCIES, when
# given, is that list (names or paths); otherwise it comes from the pkg-config file of libxml2. When
# cross-compiling, pkg-config is asked only when PKG_CONFIG_LIBDIR or PKG_CONFIG_SYSROOT_DIR is set in the
# environment (as the toolchain file of the macOS cross build does): otherwise the build host's pkg-config files,
# which describe its own libraries, would answer.
IF(NOT HUNTER_ENABLED AND NOT EMSCRIPTEN AND NOT WIN32 AND TARGET LibXml2::LibXml2 AND LIBXML2_LIBRARY MATCHES "\\.a$")
  GET_TARGET_PROPERTY(LIBXML2_INTERFACE_LINK_LIBRARIES LibXml2::LibXml2 INTERFACE_LINK_LIBRARIES)
  IF(NOT LIBXML2_INTERFACE_LINK_LIBRARIES)
    SET(LIBXML2_STATIC_HELP "set LIBXML2_STATIC_DEPENDENCIES to the libraries it needs (names or paths, for example \"icui18n;icuuc;icudata;z;lzma;m\"), or link a shared libxml2")
    IF(DEFINED LIBXML2_STATIC_DEPENDENCIES)
      SET(LIBXML2_DEPENDENCIES ${LIBXML2_STATIC_DEPENDENCIES})
      MESSAGE(STATUS "Static libxml2: linking ${LIBXML2_DEPENDENCIES}, as given by LIBXML2_STATIC_DEPENDENCIES")
    ELSE()
      IF(CMAKE_CROSSCOMPILING AND "$ENV{PKG_CONFIG_LIBDIR}" STREQUAL "" AND "$ENV{PKG_CONFIG_SYSROOT_DIR}" STREQUAL "")
        MESSAGE(FATAL_ERROR "${LIBXML2_LIBRARY} is a static libxml2, and the libraries it needs are not taken from pkg-config when cross-compiling without PKG_CONFIG_LIBDIR or PKG_CONFIG_SYSROOT_DIR in the environment, which point pkg-config to the target's files: set those, or ${LIBXML2_STATIC_HELP}.")
      ENDIF()
      FIND_PACKAGE(PkgConfig QUIET)
      IF(PKG_CONFIG_FOUND)
        PKG_CHECK_MODULES(LIBXML2_PC QUIET libxml-2.0)
      ENDIF()
      IF(NOT LIBXML2_PC_FOUND)
        MESSAGE(FATAL_ERROR "${LIBXML2_LIBRARY} is a static libxml2, and the pkg-config file libxml-2.0 that lists the libraries it needs was not found: ${LIBXML2_STATIC_HELP}.")
      ENDIF()
      # the .pc file found must be the one of that archive
      GET_FILENAME_COMPONENT(LIBXML2_ARCHIVE_DIR "${LIBXML2_LIBRARY}" REALPATH)
      GET_FILENAME_COMPONENT(LIBXML2_ARCHIVE_DIR "${LIBXML2_ARCHIVE_DIR}" DIRECTORY)
      GET_FILENAME_COMPONENT(LIBXML2_PC_REAL_LIBDIR "${LIBXML2_PC_LIBDIR}" REALPATH)
      IF(NOT LIBXML2_ARCHIVE_DIR STREQUAL LIBXML2_PC_REAL_LIBDIR AND NOT "$ENV{PKG_CONFIG_SYSROOT_DIR}" STREQUAL "")
        # pkgconf puts the sysroot in front of the libdir of a .pc file, pkg-config does not
        GET_FILENAME_COMPONENT(LIBXML2_PC_REAL_LIBDIR "$ENV{PKG_CONFIG_SYSROOT_DIR}${LIBXML2_PC_LIBDIR}" REALPATH)
      ENDIF()
      IF(NOT LIBXML2_ARCHIVE_DIR STREQUAL LIBXML2_PC_REAL_LIBDIR)
        MESSAGE(WARNING "The pkg-config file libxml-2.0 found describes the libxml2 of ${LIBXML2_PC_LIBDIR}, not ${LIBXML2_LIBRARY}: its list of libraries may not be the one that archive needs. If the link fails, ${LIBXML2_STATIC_HELP}.")
      ENDIF()
      SET(LIBXML2_DEPENDENCIES ${LIBXML2_PC_STATIC_LIBRARIES})
      LIST(REMOVE_ITEM LIBXML2_DEPENDENCIES xml2)
      IF(WITH_STATIC_EXTERNAL)
        # static externals: those libraries are looked up like the other externals, archives first; the C runtime
        # libraries are left to the linker
        SET(LIBXML2_STATIC_EXTERNAL_DEPENDENCIES)
        FOREACH(LIBXML2_DEPENDENCY ${LIBXML2_DEPENDENCIES})
          IF(LIBXML2_DEPENDENCY MATCHES "^(c|m|dl|rt|pthread)$")
            LIST(APPEND LIBXML2_STATIC_EXTERNAL_DEPENDENCIES ${LIBXML2_DEPENDENCY})
          ELSE()
            FIND_LIBRARY(LIBXML2_DEPENDENCY_${LIBXML2_DEPENDENCY}_LIBRARY ${LIBXML2_DEPENDENCY} HINTS ${LIBXML2_PC_STATIC_LIBRARY_DIRS} ${LIBXML2_PC_LIBDIR})
            IF(NOT LIBXML2_DEPENDENCY_${LIBXML2_DEPENDENCY}_LIBRARY)
              MESSAGE(FATAL_ERROR "${LIBXML2_DEPENDENCY}, a library the static libxml2 needs, was not found: ${LIBXML2_STATIC_HELP}.")
            ENDIF()
            LIST(APPEND LIBXML2_STATIC_EXTERNAL_DEPENDENCIES ${LIBXML2_DEPENDENCY_${LIBXML2_DEPENDENCY}_LIBRARY})
          ENDIF()
        ENDFOREACH()
        SET(LIBXML2_DEPENDENCIES ${LIBXML2_STATIC_EXTERNAL_DEPENDENCIES})
      ENDIF()
      MESSAGE(STATUS "Static libxml2: linking ${LIBXML2_DEPENDENCIES}, as listed by libxml-2.0.pc in ${LIBXML2_PC_LIBDIR}")
      SET_PROPERTY(TARGET LibXml2::LibXml2 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES ${LIBXML2_PC_STATIC_LIBRARY_DIRS})
      SET_PROPERTY(TARGET LibXml2::LibXml2 APPEND PROPERTY INTERFACE_LINK_OPTIONS ${LIBXML2_PC_STATIC_LDFLAGS_OTHER})
    ENDIF()
    SET_PROPERTY(TARGET LibXml2::LibXml2 APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${LIBXML2_DEPENDENCIES})
  ENDIF()
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
  ADD_COMPILE_DEFINITIONS(FINAL_VERSION=1)
ENDIF()

IF(WITH_SSE2)
  ADD_COMPILE_DEFINITIONS(NL_HAS_SSE2)
  IF(WITH_SSE3)
    ADD_COMPILE_DEFINITIONS(NL_HAS_SSE3)
  ENDIF()
ENDIF()

# The definitions of the whole build that the public NeL headers test, besides NL_DEBUG/NL_RELEASE and
# NL_HAS_SSE2/3, which NeL::misc carries itself: types_nl.h derives NL_CPU_INTEL, NL_CPU_X86_64 and NL_HAS_SSE2
# from HAVE_X86 and HAVE_X86_64, and FINAL_VERSION changes debug.h, report.h and transport_class.h. NeL::misc
# gives them to the programs built against an installed NeL, and nel-config and the .pc files print them. A
# macOS build for several architectures sets the HAVE_ ones per architecture: they are left out then.
SET(NL_PUBLIC_DEFINITIONS)
IF(NOT CMAKE_OSX_ARCHITECTURES OR TARGETS_COUNT EQUAL 1)
  IF(TARGET_X64)
    LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_X86 HAVE_X64 HAVE_X86_64)
  ELSEIF(TARGET_X86)
    LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_X86)
  ENDIF()
  IF(TARGET_ARM)
    IF(TARGET_AARCH64)
      LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_AARCH64)
    ENDIF()
    IF(TARGET_ARMV7S)
      LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_ARMV7S)
    ENDIF()
    IF(TARGET_ARMV7)
      LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_ARMV7)
    ENDIF()
    LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_ARM)
  ENDIF()
  IF(TARGET_MIPS)
    LIST(APPEND NL_PUBLIC_DEFINITIONS HAVE_MIPS)
  ENDIF()
ENDIF()
# both values: CMustConsume in debug.h has another layout in a final version
IF(FINAL_VERSION)
  LIST(APPEND NL_PUBLIC_DEFINITIONS FINAL_VERSION=1)
ELSE()
  LIST(APPEND NL_PUBLIC_DEFINITIONS FINAL_VERSION=0)
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
  # msquic's own package configuration exports the target as plain "msquic" up to 2.4.x
  # (what vcpkg ships) and as msquic::msquic since; Findmsquic.cmake provides the latter
  IF(TARGET msquic AND NOT TARGET msquic::msquic)
    ADD_LIBRARY(msquic::msquic ALIAS msquic)
  ENDIF()
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
    ENDIF()

    HUNTER_ADD_PACKAGE(CURL)
    FIND_PACKAGE(CURL CONFIG REQUIRED)
  ELSE()
    IF(EMSCRIPTEN)
      # CURL only when a cross-built (or stub) curl is provided via
      # -DCURL_INCLUDE_DIR/-DCURL_LIBRARY; requests fail gracefully with a stub.
      # find_package(CURL) can silently fail here because CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY
      # rejects any cache path outside the emsdk sysroot; when that happens,
      # manually publish the imported target from the -D cache vars so
      # nelweb/nelgui still see curl headers and stub archives.
      FIND_PACKAGE(CURL QUIET)
      IF(NOT TARGET CURL::libcurl AND CURL_INCLUDE_DIR AND CURL_LIBRARY)
        ADD_LIBRARY(CURL::libcurl STATIC IMPORTED)
        SET_TARGET_PROPERTIES(CURL::libcurl PROPERTIES
          IMPORTED_LOCATION "${CURL_LIBRARY}"
          INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}")
      ENDIF()
      # Same "empty include dir on the imported target" defense as OpenSSL below.
      IF(TARGET CURL::libcurl AND CURL_INCLUDE_DIR)
        SET_PROPERTY(TARGET CURL::libcurl APPEND PROPERTY
          INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIR}")
      ENDIF()
    ELSE()
      FIND_PACKAGE(CURL REQUIRED)
    ENDIF() # !EMSCRIPTEN
  ENDIF() #hunter

  IF(WITH_GUI)
    find_package(Luabind REQUIRED)
    find_package(Lua REQUIRED)

    if(Lua_FOUND AND NOT TARGET Lua::Lua)
      add_library(Lua::Lua INTERFACE IMPORTED)
      set_target_properties(
              Lua::Lua
              PROPERTIES
              INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
              INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
      )
    endif()
  ENDIF()

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/nel/include)
  ADD_SUBDIRECTORY(nel)

  # allow find_package to find projects from within the mono repo
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/nel/internal")
ENDIF()

# Without WITH_NEL, each project below finds the installed NeL with the components it uses. The imported NeL targets
# and those of the dependencies of the components then share that project's directory, where the link interfaces of
# the NeL targets are resolved, and a project without the GUI needs no Lua and Luabind.

IF(WITH_RYZOM)
  ADD_SUBDIRECTORY(ryzom)
ENDIF()

IF(WITH_NELNS)
  ADD_SUBDIRECTORY(nelns)
ENDIF()

IF(WITH_SNOWBALLS)
  ADD_SUBDIRECTORY(snowballs2)
ENDIF()

IF(WITH_TOOLS OR WITH_RYZOM_TOOLS)
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
OPTION(BUILD_DOCUMENTATION "Build the NeL documentation with Doxygen (target DoxygenDoc)" OFF)
IF(BUILD_DOCUMENTATION)
    FIND_PACKAGE(Doxygen OPTIONAL_COMPONENTS dot)
    IF(NOT DOXYGEN_FOUND)
        MESSAGE(FATAL_ERROR "BUILD_DOCUMENTATION needs Doxygen, which was not found: install it or turn BUILD_DOCUMENTATION off")
    ENDIF()
    IF(TARGET Doxygen::dot)
        SET(HAVE_DOT YES)
    ELSE()
        SET(HAVE_DOT NO)
    ENDIF()
    # This processes our Doxyfile.in and substitutes paths to generate
    # a final Doxyfile
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/nel/doc/Doxyfile.cmake.in ${CMAKE_BINARY_DIR}/doc/Doxyfile)

    ADD_CUSTOM_TARGET(DoxygenDoc COMMAND Doxygen::doxygen ${CMAKE_BINARY_DIR}/doc/Doxyfile WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
ENDIF()

IF(WITH_NEL_TESTS)
  ENABLE_TESTING()
  IF(BUILD_DASHBOARD)
    INCLUDE(Dart)
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
SET(CPACK_NSIS_HELP_LINK "https:\\\\\\\\wiki.ryzom.dev")
SET(CPACK_NSIS_URL_INFO_ABOUT "https:\\\\\\\\wiki.ryzom.dev")
SET(CPACK_NSIS_CONTACT "https://wiki.ryzom.dev/")

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
