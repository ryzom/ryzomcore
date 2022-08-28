# Force Release configuration for compiler checks
SET(CMAKE_TRY_COMPILE_CONFIGURATION "Release")

# Force Release configuration by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
ENDIF()

# Declare CMAKE_CONFIGURATION_TYPES before PROJECT
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

###
# Helper macro that generates .pc and installs it.
# Argument: name - the name of the .pc package, e.g. "nel-pacs.pc"
###
MACRO(NL_GEN_PC name)
  IF(NOT WIN32 AND WITH_INSTALL_LIBRARIES)
    CONFIGURE_FILE(${name}.in "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}" DESTINATION ${NL_LIB_PREFIX}/pkgconfig)
  ENDIF()
ENDMACRO(NL_GEN_PC)

###
#
###
MACRO(NL_TARGET_LIB name)
  IF(WITH_STATIC)
    ADD_LIBRARY(${name} STATIC ${ARGN})
  ELSE()
    ADD_LIBRARY(${name} SHARED ${ARGN})
  ENDIF()
ENDMACRO(NL_TARGET_LIB)

###
#
###
MACRO(NL_TARGET_DRIVER name)
  IF(WITH_STATIC_DRIVERS)
    ADD_LIBRARY(${name} STATIC ${ARGN})
  ELSE()
    ADD_LIBRARY(${name} MODULE ${ARGN})
  ENDIF()
ENDMACRO(NL_TARGET_DRIVER)

###
# Helper macro that sets the default library properties.
# Argument: name - the target name whose properties are being set
# Argument:
###
MACRO(NL_DEFAULT_PROPS name label)
  IF(TARGET revision)
    # explicitly say that the target depends on revision.h
    ADD_DEPENDENCIES(${name} revision)
  ENDIF()

  # Note: This is just a workaround for a CMake bug generating VS10 files with a colon in the project name.
  # CMake Bug ID: http://www.cmake.org/Bug/view.php?id=11819
  STRING(REGEX REPLACE "\\:" " -" proj_label ${label})
  SET_TARGET_PROPERTIES(${name} PROPERTIES PROJECT_LABEL ${proj_label})
  GET_TARGET_PROPERTY(type ${name} TYPE)
  IF(${type} STREQUAL SHARED_LIBRARY)
    # Set versions only if target is a shared library
    SET_TARGET_PROPERTIES(${name} PROPERTIES
      VERSION ${NL_VERSION} SOVERSION ${NL_VERSION_MAJOR})
    IF(NL_LIB_PREFIX)
      SET_TARGET_PROPERTIES(${name} PROPERTIES INSTALL_NAME_DIR ${NL_LIB_PREFIX})
    ENDIF()
  ENDIF()

  IF(${type} STREQUAL EXECUTABLE AND WIN32 AND NOT MINGW)
    # check if using a GUI
    GET_TARGET_PROPERTY(_VALUE ${name} WIN32_EXECUTABLE)

    IF(TARGET_X64)
      # Target Windows XP 64 bits
      SET(_SUBSYSTEM_VERSION "5.02")
    ELSE()
      # Target Windows XP
      SET(_SUBSYSTEM_VERSION "5.01")
    ENDIF()

    IF(_VALUE)
      # GUI
      SET(_SUBSYSTEM "WINDOWS")
    ELSE()
      # Console
      SET(_SUBSYSTEM "CONSOLE")
    ENDIF()

    SET_TARGET_PROPERTIES(${name} PROPERTIES
      VERSION ${NL_VERSION}
      SOVERSION ${NL_VERSION_MAJOR}
      COMPILE_FLAGS "/GA"
      LINK_FLAGS "/VERSION:${NL_VERSION_MAJOR}.${NL_VERSION_MINOR} /SUBSYSTEM:${_SUBSYSTEM},${_SUBSYSTEM_VERSION}")
  ENDIF()
ENDMACRO(NL_DEFAULT_PROPS)

###
# Adds the target suffix on Windows.
# Argument: name - the library's target name.
###
MACRO(NL_ADD_LIB_SUFFIX name)
  IF(WIN32)
    SET_TARGET_PROPERTIES(${name} PROPERTIES DEBUG_POSTFIX "_d" RELEASE_POSTFIX "_r")
  ENDIF()
ENDMACRO(NL_ADD_LIB_SUFFIX)

###
# Adds the runtime link flags for Win32 binaries and links STLport.
# Argument: name - the target to add the link flags to.
###
MACRO(NL_ADD_RUNTIME_FLAGS name)
  IF(WIN32)
#    SET_TARGET_PROPERTIES(${name} PROPERTIES
#      LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
#      LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}")
  ENDIF()
ENDMACRO(NL_ADD_RUNTIME_FLAGS)

###
# Checks build vs. source location. Prevents In-Source builds.
###
MACRO(CHECK_OUT_OF_SOURCE)
  IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "

CMake generation for this project is not allowed within the source directory!
Remove the CMakeCache.txt file and try again from another folder, e.g.:

   rm CMakeCache.txt
   mkdir cmake
   cd cmake
   cmake ..
    ")
  ENDIF()

ENDMACRO(CHECK_OUT_OF_SOURCE)

MACRO(NL_SETUP_DEFAULT_OPTIONS)
  ###
  # Features
  ###
  OPTION(WITH_LOGGING             "With Logging"                                  ON )
  OPTION(WITH_COVERAGE            "With Code Coverage Support"                    OFF)
  OPTION(WITH_PCH                 "With Precompiled Headers"                      ON )
  OPTION(WITH_LOW_MEMORY          "With low memory (use the least of RAM)"        OFF)
  OPTION(FINAL_VERSION            "Build in Final Version mode"                   ON )

  # Default to static building on Windows.
  IF(WIN32)
    OPTION(WITH_STATIC            "With static libraries."                        ON )
  ELSE()
    OPTION(WITH_STATIC            "With static libraries."                        OFF)
  ENDIF()
  IF (WITH_STATIC)
    OPTION(WITH_STATIC_LIBXML2    "With static libxml2"                           ON )
  ELSE()
    OPTION(WITH_STATIC_LIBXML2    "With static libxml2"                           OFF)
  ENDIF()
  IF (WITH_STATIC)
    OPTION(WITH_STATIC_CURL       "With static curl"                              ON )
  ELSE()
    OPTION(WITH_STATIC_CURL       "With static curl"                              OFF)
  ENDIF()
  IF(APPLE)
    OPTION(WITH_LIBXML2_ICONV     "With libxml2 using iconv"                      ON )
  ELSE()
    OPTION(WITH_LIBXML2_ICONV     "With libxml2 using iconv"                      OFF)
  ENDIF()
  OPTION(WITH_STATIC_DRIVERS      "With static drivers."                          OFF)
  IF(WIN32)
    OPTION(WITH_EXTERNAL          "With provided external."                       ON )
  ELSE()
    OPTION(WITH_EXTERNAL          "With provided external."                       OFF)
  ENDIF()
  OPTION(WITH_STATIC_EXTERNAL     "With static external libraries"                OFF)
  OPTION(WITH_STATIC_RUNTIMES     "Use only static C++ runtimes"                  OFF)
  IF(UNIX AND NOT APPLE)
    OPTION(WITH_UNIX_STRUCTURE    "Use UNIX structure (bin, include, lib)"        ON )
  ELSE()
    OPTION(WITH_UNIX_STRUCTURE    "Use UNIX structure (bin, include, lib)"        OFF)
  ENDIF()
  OPTION(WITH_INSTALL_LIBRARIES   "Install development files."                    ON )

  ###
  # GUI toolkits
  ###
  OPTION(WITH_GTK                 "With GTK Support"                              OFF)

  IF(WIN32 AND MFC_FOUND)
    OPTION(WITH_MFC               "With MFC Support"                              ON )
  ELSE()
    OPTION(WITH_MFC               "With MFC Support"                              OFF)
  ENDIF()

  ###
  # Optional support
  ###
  OPTION(WITH_SYMBOLS             "Keep debug symbols in binaries"                OFF)

  OPTION(BUILD_DASHBOARD          "Build to the CDash dashboard"                  OFF)
ENDMACRO(NL_SETUP_DEFAULT_OPTIONS)

MACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)
  OPTION(WITH_SSE2                "With SSE2"                                     ON )
  OPTION(WITH_SSE3                "With SSE3"                                     ON )

  IF(NOT MSVC)
    OPTION(WITH_GCC_FPMATH_BOTH   "With GCC -mfpmath=both"                        OFF)
  ENDIF()
ENDMACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)

MACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_RYZOM_TOOLS         "Build Ryzom Core Tools"                        ON )
ENDMACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)

MACRO(ADD_PLATFORM_FLAGS _FLAGS)
  SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} ${_FLAGS}")
  SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} ${_FLAGS}")
ENDMACRO()

MACRO(ADD_PLATFORM_LINKFLAGS _FLAGS)
  SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} ${_FLAGS}")
ENDMACRO()

MACRO(CONVERT_VERSION_NUMBER _VERSION_NUMBER _BASE)
  SET(${_VERSION_NUMBER} 0)
  FOREACH(_ARG ${ARGN})
    MATH(EXPR ${_VERSION_NUMBER} "${${_VERSION_NUMBER}} * ${_BASE} + ${_ARG}")
  ENDFOREACH()
ENDMACRO()

MACRO(NL_SETUP_BUILD)
  #-----------------------------------------------------------------------------
  # Setup the buildmode variables.
  #
  # None                  = NL_RELEASE
  # Debug                 = NL_DEBUG
  # Release               = NL_RELEASE

  IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(NL_BUILD_MODE "NL_DEBUG")
  ELSE()
    IF(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
    ELSE()
      SET(NL_BUILD_MODE "NL_RELEASE")
      # enforce release mode if it's neither Debug nor Release
      SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    ENDIF()
  ENDIF()

  IF(CMAKE_CXX_LIBRARY_ARCHITECTURE)
    SET(HOST_CPU ${CMAKE_CXX_LIBRARY_ARCHITECTURE})
  ELSE()
    SET(HOST_CPU ${CMAKE_HOST_SYSTEM_PROCESSOR})
  ENDIF()

  IF(HOST_CPU MATCHES "(amd|AMD|x86_)64")
    SET(HOST_CPU "x86_64")
  ELSEIF(HOST_CPU MATCHES "i.86")
    SET(HOST_CPU "x86")
  ENDIF()

  # Determine target CPU

  # If not specified, use the same CPU as host
  IF(NOT TARGET_CPU)
    SET(TARGET_CPU ${HOST_CPU})
  ENDIF()

  IF(TARGET_CPU MATCHES "(amd|AMD|x86_)64")
    SET(TARGET_CPU "x86_64")
  ELSEIF(TARGET_CPU MATCHES "i.86")
    SET(TARGET_CPU "x86")
  ENDIF()

  IF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    SET(CLANG ON)
    MESSAGE(STATUS "Using Clang ${CMAKE_CXX_COMPILER_VERSION} compiler")
  ENDIF()

  IF(CMAKE_GENERATOR MATCHES "Xcode")
    SET(XCODE ON)
    MESSAGE(STATUS "Generating Xcode project")
  ENDIF()

  IF(CMAKE_GENERATOR MATCHES "NMake")
    SET(NMAKE ON)
    MESSAGE(STATUS "Generating NMake project")
  ENDIF()

  IF(CMAKE_GENERATOR MATCHES "Ninja")
    SET(NINJA ON)
    MESSAGE(STATUS "Generating Ninja project")
  ENDIF()

  # If target and host CPU are the same
  IF("${HOST_CPU}" STREQUAL "${TARGET_CPU}" AND NOT CMAKE_CROSSCOMPILING)
    # x86-compatible CPU
    IF(HOST_CPU MATCHES "x86")
      IF(NOT CMAKE_SIZEOF_VOID_P)
        INCLUDE (CheckTypeSize)
        CHECK_TYPE_SIZE("void*"  CMAKE_SIZEOF_VOID_P)
      ENDIF()

      # Using 32 or 64 bits libraries
      IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(TARGET_CPU "x86_64")
      ELSE()
        SET(TARGET_CPU "x86")
      ENDIF()
    ELSEIF(HOST_CPU MATCHES "arm")
      SET(TARGET_CPU "arm")
    ELSE()
      SET(TARGET_CPU "unknown")
      MESSAGE(STATUS "Unknown architecture: ${HOST_CPU}")
    ENDIF()
    # TODO: add checks for PPC
  ELSE()
    MESSAGE(STATUS "Compiling on ${HOST_CPU} for ${TARGET_CPU}")
  ENDIF()

  # Use values from environment variables
  SET(PLATFORM_CFLAGS "$ENV{CFLAGS} $ENV{CPPFLAGS} ${PLATFORM_CFLAGS}")
  SET(PLATFORM_CXXFLAGS "$ENV{CXXFLAGS} $ENV{CPPFLAGS} ${PLATFORM_CXXFLAGS}")
  SET(PLATFORM_LINKFLAGS "$ENV{LDFLAGS} ${PLATFORM_LINKFLAGS}")

  # Remove -g and -O flag because we are managing them ourself
  STRING(REPLACE "-g" "" PLATFORM_CFLAGS ${PLATFORM_CFLAGS})
  STRING(REPLACE "-g" "" PLATFORM_CXXFLAGS ${PLATFORM_CXXFLAGS})
  STRING(REGEX REPLACE "-O[0-9s]" "" PLATFORM_CFLAGS ${PLATFORM_CFLAGS})
  STRING(REGEX REPLACE "-O[0-9s]" "" PLATFORM_CXXFLAGS ${PLATFORM_CXXFLAGS})

  # Strip spaces
  STRING(STRIP ${PLATFORM_CFLAGS} PLATFORM_CFLAGS)
  STRING(STRIP ${PLATFORM_CXXFLAGS} PLATFORM_CXXFLAGS)
  STRING(STRIP ${PLATFORM_LINKFLAGS} PLATFORM_LINKFLAGS)

  IF(NOT CMAKE_OSX_ARCHITECTURES)
    IF(TARGET_CPU STREQUAL "x86_64")
      SET(TARGET_X64 1)
      SET(TARGET_X86 1)
    ELSEIF(TARGET_CPU STREQUAL "x86")
      SET(TARGET_X86 1)
    ELSEIF(TARGET_CPU STREQUAL "arm64")
      SET(TARGET_ARM 1)
      SET(TARGET_ARM64 1)
    ELSEIF(TARGET_CPU STREQUAL "armv7s")
      SET(TARGET_ARM 1)
      SET(TARGET_ARMV7S 1)
    ELSEIF(TARGET_CPU STREQUAL "armv7")
      SET(TARGET_ARM 1)
      SET(TARGET_ARMV7 1)
    ELSEIF(TARGET_CPU STREQUAL "armv6")
      SET(TARGET_ARM 1)
      SET(TARGET_ARMV6 1)
    ELSEIF(TARGET_CPU STREQUAL "armv5")
      SET(TARGET_ARM 1)
      SET(TARGET_ARMV5 1)
    ELSEIF(TARGET_CPU STREQUAL "arm")
      SET(TARGET_ARM 1)
    ELSEIF(TARGET_CPU STREQUAL "mips")
      SET(TARGET_MIPS 1)
    ENDIF()

    IF(TARGET_ARM)
      IF(TARGET_ARM64)
        ADD_PLATFORM_FLAGS("-DHAVE_ARM64")
      ENDIF()

      IF(TARGET_ARMV7S)
        ADD_PLATFORM_FLAGS("-DHAVE_ARMV7S")
      ENDIF()

      IF(TARGET_ARMV7)
        ADD_PLATFORM_FLAGS("-DHAVE_ARMV7")
      ENDIF()

      IF(TARGET_ARMV6)
        ADD_PLATFORM_FLAGS("-HAVE_ARMV6")
      ENDIF()

      ADD_PLATFORM_FLAGS("-DHAVE_ARM")
    ENDIF()

    IF(TARGET_X86)
      ADD_PLATFORM_FLAGS("-DHAVE_X86")
    ENDIF()

    IF(TARGET_X64)
      ADD_PLATFORM_FLAGS("-DHAVE_X64 -DHAVE_X86_64")
    ENDIF()

    IF(TARGET_MIPS)
      ADD_PLATFORM_FLAGS("-DHAVE_MIPS")
    ENDIF()
  ENDIF()

  # Fix library paths suffixes for Debian MultiArch
  IF(LIBRARY_ARCHITECTURE)
    SET(CMAKE_LIBRARY_PATH /lib/${LIBRARY_ARCHITECTURE} /usr/lib/${LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_PATH})
    IF(TARGET_X64)
      SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib64 /usr/lib64)
    ELSEIF(TARGET_X86)
      SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib32 /usr/lib32)
    ENDIF()
  ENDIF()

  IF(APPLE AND NOT IOS)
    SET(CMAKE_INCLUDE_PATH /opt/local/include ${CMAKE_INCLUDE_PATH})
    SET(CMAKE_LIBRARY_PATH /opt/local/lib ${CMAKE_LIBRARY_PATH})
  ENDIF()

  IF(WITH_LOGGING)
    ADD_PLATFORM_FLAGS("-DENABLE_LOGS")
  ENDIF()

  IF(MSVC)
    # Ignore default include paths
    ADD_PLATFORM_FLAGS("/X")

    IF(MSVC14)
      ADD_PLATFORM_FLAGS("/Gy-")
      # /Ox is working with VC++ 2015 and 2017, but custom optimizations don't exist
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 /GF- ${DEBUG_CFLAGS}")

      # Special cases for VC++ 2017
      IF(MSVC_VERSION EQUAL "1911")
        SET(MSVC1411 ON)
      ELSEIF(MSVC_VERSION EQUAL "1910")
        SET(MSVC1410 ON)
      ENDIF()
    ELSEIF(MSVC12)
      ADD_PLATFORM_FLAGS("/Gy-")
      # /Ox is working with VC++ 2013, but custom optimizations don't exist
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 /GF- ${DEBUG_CFLAGS}")
    ELSEIF(MSVC11)
      ADD_PLATFORM_FLAGS("/Gy-")
      # /Ox is working with VC++ 2012, but custom optimizations don't exist
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 /GF- ${DEBUG_CFLAGS}")
    ELSEIF(MSVC10)
      ADD_PLATFORM_FLAGS("/Gy-")
      # /Ox is working with VC++ 2010, but custom optimizations don't exist
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 /GF- ${DEBUG_CFLAGS}")
    ELSEIF(MSVC90)
      ADD_PLATFORM_FLAGS("/Gy-")
      # don't use a /O[012x] flag if you want custom optimizations
      SET(RELEASE_CFLAGS "/Ob2 /Oi /Ot /Oy /GT /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Ob1 /GF- ${DEBUG_CFLAGS}")
    ELSEIF(MSVC80)
      ADD_PLATFORM_FLAGS("/Gy- /Wp64")
      # don't use a /O[012x] flag if you want custom optimizations
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 ${DEBUG_CFLAGS}")
    ELSE()
      MESSAGE(FATAL_ERROR "Can't determine compiler version ${MSVC_VERSION}")
    ENDIF()

    ADD_PLATFORM_FLAGS("/D_CRT_SECURE_NO_DEPRECATE /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /D_WIN32 /DWIN32 /D_WINDOWS /wd4250")

    # huge PCH
    ADD_PLATFORM_FLAGS("/Zm1000")

    IF(TARGET_X64)
      # Fix a bug with Intellisense
      ADD_PLATFORM_FLAGS("/D_WIN64")
      # Fix a compilation error for some big C++ files
      ADD_PLATFORM_FLAGS("/bigobj")
    ELSE()
      # Allows 32 bits applications to use 3 GB of RAM
      ADD_PLATFORM_LINKFLAGS("/LARGEADDRESSAWARE")
    ENDIF()

    # Exceptions are only set for C++
    SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} /EHa")

    IF(WITH_SYMBOLS)
      SET(NL_RELEASE_CFLAGS "/Zi ${NL_RELEASE_CFLAGS}")
      SET(NL_RELEASE_LINKFLAGS "/DEBUG ${NL_RELEASE_LINKFLAGS}")
    ELSE()
      SET(NL_RELEASE_LINKFLAGS "/RELEASE ${NL_RELEASE_LINKFLAGS}")
    ENDIF()

    IF(WITH_STATIC_RUNTIMES)
      SET(RUNTIME_FLAG "/MT")
    ELSE()
      SET(RUNTIME_FLAG "/MD")
    ENDIF()

    SET(NL_DEBUG_CFLAGS "/Zi ${RUNTIME_FLAG}d /RTC1 /D_DEBUG ${DEBUG_CFLAGS} ${NL_DEBUG_CFLAGS}")
    SET(NL_RELEASE_CFLAGS "${RUNTIME_FLAG} /DNDEBUG ${RELEASE_CFLAGS} ${NL_RELEASE_CFLAGS}")
    SET(NL_DEBUG_LINKFLAGS "/DEBUG /OPT:NOREF /OPT:NOICF /NODEFAULTLIB:msvcrt ${MSVC_INCREMENTAL_YES_FLAG} ${NL_DEBUG_LINKFLAGS}")
    SET(NL_RELEASE_LINKFLAGS "/OPT:REF /OPT:ICF /INCREMENTAL:NO ${NL_RELEASE_LINKFLAGS}")

    IF(WITH_WARNINGS)
      SET(DEBUG_CFLAGS "/W4 ${DEBUG_CFLAGS}")
    ELSE()
      SET(DEBUG_CFLAGS "/W3 ${DEBUG_CFLAGS}")
    ENDIF()
  ELSE()
    IF(WIN32)
      ADD_PLATFORM_FLAGS("-DWIN32 -D_WIN32")

      IF(CLANG)
        ADD_PLATFORM_FLAGS("-nobuiltininc")
      ENDIF()
    ENDIF()

    IF(WITH_SSE3)
      ADD_PLATFORM_FLAGS("-msse3")
    ENDIF()

    IF(WITH_GCC_FPMATH_BOTH)
      ADD_PLATFORM_FLAGS("-mfpmath=both")
    ENDIF()

    IF(APPLE)
      SET(OBJC_FLAGS -fobjc-abi-version=2 -fobjc-legacy-dispatch -fobjc-weak)

      IF(NOT XCODE)
        IF(CMAKE_OSX_ARCHITECTURES)
          SET(TARGETS_COUNT 0)
          SET(_ARCHS)
          FOREACH(_ARCH ${CMAKE_OSX_ARCHITECTURES})
            IF(_ARCH STREQUAL "i386")
              SET(_ARCHS "${_ARCHS} i386")
              SET(TARGET_X86 1)
              MATH(EXPR TARGETS_COUNT "${TARGETS_COUNT}+1")
            ELSEIF(_ARCH STREQUAL "x86_64")
              SET(_ARCHS "${_ARCHS} x86_64")
              SET(TARGET_X64 1)
              MATH(EXPR TARGETS_COUNT "${TARGETS_COUNT}+1")
            ELSEIF(_ARCH STREQUAL "armv7s")
              SET(_ARCHS "${_ARCHS} armv7s")
              SET(TARGET_ARMV7S 1)
              SET(TARGET_ARM 1)
              MATH(EXPR TARGETS_COUNT "${TARGETS_COUNT}+1")
            ELSEIF(_ARCH STREQUAL "armv7")
              SET(_ARCHS "${_ARCHS} armv7")
              SET(TARGET_ARMV7 1)
              SET(TARGET_ARM 1)
              MATH(EXPR TARGETS_COUNT "${TARGETS_COUNT}+1")
            ELSEIF(_ARCH STREQUAL "armv6")
              SET(_ARCHS "${_ARCHS} armv6")
              SET(TARGET_ARMV6 1)
              SET(TARGET_ARM 1)
              MATH(EXPR TARGETS_COUNT "${TARGETS_COUNT}+1")
            ELSEIF(_ARCH STREQUAL "mips")
              SET(_ARCHS "${_ARCHS} mips")
              SET(TARGET_MIPS 1)
              MATH(EXPR TARGETS_COUNT "${TARGETS_COUNT}+1")
            ELSE()
              SET(_ARCHS "${_ARCHS} unknwon(${_ARCH})")
            ENDIF()
          ENDFOREACH(_ARCH)
          MESSAGE(STATUS "Compiling under Mac OS X for ${TARGETS_COUNT} architectures: ${_ARCHS}")
        ELSE()
          SET(TARGETS_COUNT 0)
        ENDIF()

        IF(TARGETS_COUNT EQUAL 1)
          IF(TARGET_ARM)
            IF(TARGET_ARMV7S)
              ADD_PLATFORM_FLAGS("-arch armv7s -DHAVE_ARMV7S")
            ENDIF()

            IF(TARGET_ARMV7)
              ADD_PLATFORM_FLAGS("-arch armv7 -DHAVE_ARMV7")
            ENDIF()

            IF(TARGET_ARMV6)
              ADD_PLATFORM_FLAGS("-arch armv6 -DHAVE_ARMV6")
            ENDIF()

            IF(TARGET_ARMV5)
              ADD_PLATFORM_FLAGS("-arch armv5 -DHAVE_ARMV5")
            ENDIF()

            ADD_PLATFORM_FLAGS("-mthumb -DHAVE_ARM")
          ENDIF()

          IF(TARGET_X64)
            ADD_PLATFORM_FLAGS("-arch x86_64 -DHAVE_X64 -DHAVE_X86_64 -DHAVE_X86")
          ELSEIF(TARGET_X86)
            ADD_PLATFORM_FLAGS("-arch i386 -DHAVE_X86")
          ENDIF()

          IF(TARGET_MIPS)
            ADD_PLATFORM_FLAGS("-arch mips -DHAVE_MIPS")
          ENDIF()
        ELSEIF(TARGETS_COUNT EQUAL 0)
          # Not using CMAKE_OSX_ARCHITECTURES, HAVE_XXX already defined before
          IF(TARGET_ARM)
            IF(TARGET_ARMV7S)
              ADD_PLATFORM_FLAGS("-arch armv7s")
            ENDIF()

            IF(TARGET_ARMV7)
              ADD_PLATFORM_FLAGS("-arch armv7")
            ENDIF()

            IF(TARGET_ARMV6)
              ADD_PLATFORM_FLAGS("-arch armv6")
            ENDIF()

            IF(TARGET_ARMV5)
              ADD_PLATFORM_FLAGS("-arch armv5")
            ENDIF()

            ADD_PLATFORM_FLAGS("-mthumb")
          ENDIF()

          IF(TARGET_X64)
            ADD_PLATFORM_FLAGS("-arch x86_64")
          ELSEIF(TARGET_X86)
            ADD_PLATFORM_FLAGS("-arch i386")
          ENDIF()

          IF(TARGET_MIPS)
            ADD_PLATFORM_FLAGS("-arch mips")
          ENDIF()
        ELSE()
          IF(TARGET_ARMV6)
            ADD_PLATFORM_FLAGS("-Xarch_armv6 -mthumb -Xarch_armv6 -DHAVE_ARM -Xarch_armv6 -DHAVE_ARMV6")
          ENDIF()

          IF(TARGET_ARMV7)
            ADD_PLATFORM_FLAGS("-Xarch_armv7 -mthumb -Xarch_armv7 -DHAVE_ARM -Xarch_armv7 -DHAVE_ARMV7")
          ENDIF()

          IF(TARGET_X86)
            ADD_PLATFORM_FLAGS("-Xarch_i386 -DHAVE_X86")
          ENDIF()

          IF(TARGET_X64)
            ADD_PLATFORM_FLAGS("-Xarch_x86_64 -DHAVE_X64 -Xarch_x86_64 -DHAVE_X86_64")
          ENDIF()

          IF(TARGET_MIPS)
            ADD_PLATFORM_FLAGS("-Xarch_mips -DHAVE_MIPS")
          ENDIF()
        ENDIF()

        IF(IOS)
          SET(CMAKE_OSX_SYSROOT "" CACHE PATH "" FORCE)

          IF(IOS_VERSION)
            PARSE_VERSION_STRING(${IOS_VERSION} IOS_VERSION_MAJOR IOS_VERSION_MINOR IOS_VERSION_PATCH)
            CONVERT_VERSION_NUMBER(${IOS_VERSION_MAJOR} ${IOS_VERSION_MINOR} ${IOS_VERSION_PATCH} IOS_VERSION_NUMBER)

            ADD_PLATFORM_FLAGS("-D__IPHONE_OS_VERSION_MIN_REQUIRED=${IOS_VERSION_NUMBER}")
          ENDIF()

          IF(CMAKE_IOS_SYSROOT)
            IF(TARGET_ARMV7S)
              IF(TARGETS_COUNT GREATER 1)
                SET(XARCH "-Xarch_armv7s ")
              ENDIF()

              ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SYSROOT}")
              ADD_PLATFORM_FLAGS("${XARCH}-miphoneos-version-min=${IOS_VERSION}")
              ADD_PLATFORM_LINKFLAGS("${XARCH}-Wl,-iphoneos_version_min,${IOS_VERSION}")
            ENDIF()

            IF(TARGET_ARMV7)
              IF(TARGETS_COUNT GREATER 1)
                SET(XARCH "-Xarch_armv7 ")
              ENDIF()

              ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SYSROOT}")
              ADD_PLATFORM_FLAGS("${XARCH}-miphoneos-version-min=${IOS_VERSION}")
              ADD_PLATFORM_LINKFLAGS("${XARCH}-Wl,-iphoneos_version_min,${IOS_VERSION}")
            ENDIF()

            IF(TARGET_ARMV6)
              IF(TARGETS_COUNT GREATER 1)
                SET(XARCH "-Xarch_armv6 ")
              ENDIF()

              ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SYSROOT}")
              ADD_PLATFORM_FLAGS("${XARCH}-miphoneos-version-min=${IOS_VERSION}")
              ADD_PLATFORM_LINKFLAGS("${XARCH}-Wl,-iphoneos_version_min,${IOS_VERSION}")
            ENDIF()
          ENDIF()

          IF(CMAKE_IOS_SIMULATOR_SYSROOT AND TARGET_X86)
            IF(TARGETS_COUNT GREATER 1)
              SET(XARCH "-Xarch_i386 ")
            ENDIF()

            ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SIMULATOR_SYSROOT}")
            ADD_PLATFORM_FLAGS("${XARCH}-mios-simulator-version-min=${IOS_VERSION}")
            IF(CMAKE_OSX_DEPLOYMENT_TARGET)
              ADD_PLATFORM_LINKFLAGS("${XARCH}-Wl,-macosx_version_min,${CMAKE_OSX_DEPLOYMENT_TARGET}")
            ENDIF()
          ENDIF()
        ELSE()
          # Always force -mmacosx-version-min to override environement variable
          IF(CMAKE_OSX_DEPLOYMENT_TARGET)
            IF(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "10.7")
              MESSAGE(FATAL_ERROR "Minimum target for OS X is 10.7 but you're using ${CMAKE_OSX_DEPLOYMENT_TARGET}")
            ENDIF()
            ADD_PLATFORM_LINKFLAGS("-Wl,-macosx_version_min,${CMAKE_OSX_DEPLOYMENT_TARGET}")
          ENDIF()
        ENDIF()

        # use libc++ under OX X to be able to use new C++ features (and else it'll use GCC 4.2.1 STL)
        # minimum target is now OS X 10.7
        SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} -stdlib=libc++")

        ADD_PLATFORM_LINKFLAGS("-Wl,-headerpad_max_install_names")

        IF(HAVE_FLAG_SEARCH_PATHS_FIRST)
          ADD_PLATFORM_LINKFLAGS("-Wl,-search_paths_first")
        ENDIF()
      ENDIF()
    ELSE()
      IF(HOST_CPU STREQUAL "x86_64" AND TARGET_CPU STREQUAL "x86")
        ADD_PLATFORM_FLAGS("-m32 -march=i686")
      ENDIF()

      IF(HOST_CPU STREQUAL "x86" AND TARGET_CPU STREQUAL "x86_64")
        ADD_PLATFORM_FLAGS("-m64")
      ENDIF()
    ENDIF()

    # use c++0x standard to use std::unique_ptr and std::shared_ptr
    IF(CMAKE_CXX11_EXTENSION_COMPILE_OPTION)
      SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} ${CMAKE_CXX11_EXTENSION_COMPILE_OPTION}")
    ENDIF()

    ADD_PLATFORM_FLAGS("-D_REENTRANT")

    # hardening
    ADD_PLATFORM_FLAGS("-D_FORTIFY_SOURCE=2")

    IF(NOT WITH_LOW_MEMORY)
      ADD_PLATFORM_FLAGS("-pipe")
    ENDIF()

    IF(WITH_COVERAGE)
      ADD_PLATFORM_FLAGS("-fprofile-arcs -ftest-coverage")
    ENDIF()

    IF(WITH_WARNINGS)
      ADD_PLATFORM_FLAGS("-Wall")
    ELSE()
      # Check wrong formats in printf-like functions
      ADD_PLATFORM_FLAGS("-Wformat -Werror=format-security")
    ENDIF()

    # never display these warnings because they are minor
    ADD_PLATFORM_FLAGS("-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-value")

    IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "6.0.0")
      ADD_PLATFORM_FLAGS("-Wno-unused-local-typedefs")
    ELSEIF(CLANG)
      ADD_PLATFORM_FLAGS("-Wno-unused-private-field -Wno-unused-local-typedef")
    ENDIF()

    IF(ANDROID)
      ADD_PLATFORM_FLAGS("--sysroot=${PLATFORM_ROOT}")
      ADD_PLATFORM_FLAGS("-ffunction-sections -funwind-tables")
      ADD_PLATFORM_FLAGS("-DANDROID")
      ADD_PLATFORM_FLAGS("-Wa,--noexecstack")

      IF(TARGET_ARM)
        ADD_PLATFORM_FLAGS("-fpic")
        ADD_PLATFORM_FLAGS("-D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__")

        IF(TARGET_ARMV7)
          ADD_PLATFORM_FLAGS("-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16")
          ADD_PLATFORM_LINKFLAGS("-march=armv7-a -Wl,--fix-cortex-a8")
        ELSEIF(TARGET_ARMV5)
          ADD_PLATFORM_FLAGS("-march=armv5te -mtune=xscale -msoft-float")
        ENDIF()

        SET(TARGET_THUMB ON)
        IF(TARGET_THUMB)
          ADD_PLATFORM_FLAGS("-mthumb -finline-limit=64")
          SET(DEBUG_CFLAGS "${DEBUG_CFLAGS} -marm")
        ELSE()
          ADD_PLATFORM_FLAGS("-funswitch-loops -finline-limit=300")
        ENDIF()
      ELSEIF(TARGET_X86)
        # Optimizations for Intel Atom
        ADD_PLATFORM_FLAGS("-march=i686 -mtune=atom -mstackrealign -msse3 -mfpmath=sse -m32 -flto -ffast-math -funroll-loops")
        ADD_PLATFORM_FLAGS("-funswitch-loops -finline-limit=300")
      ELSEIF(TARGET_MIPS)
        ADD_PLATFORM_FLAGS("-fpic -finline-functions -fmessage-length=0 -fno-inline-functions-called-once -fgcse-after-reload -frerun-cse-after-loop -frename-registers")
        SET(RELEASE_CFLAGS "${RELEASE_CFLAGS} -funswitch-loops -finline-limit=300")
      ENDIF()
      ADD_PLATFORM_LINKFLAGS("-Wl,-z,noexecstack")
      ADD_PLATFORM_LINKFLAGS("-L${PLATFORM_ROOT}/usr/lib")
    ENDIF()

    IF(APPLE)
      ADD_PLATFORM_FLAGS("-gdwarf-2 -D_DARWIN_UNLIMITED_STREAMS")
    ENDIF()

    # Fix "relocation R_X86_64_32 against.." error on x64 platforms
    IF(NOT MINGW)
      ADD_PLATFORM_FLAGS("-fPIC")
    ENDIF()

    # hardening
    ADD_PLATFORM_FLAGS("-fstack-protector --param=ssp-buffer-size=4")

    # If -fstack-protector or -fstack-protector-all enabled, enable too new warnings and fix possible link problems
    IF(WITH_WARNINGS)
      ADD_PLATFORM_FLAGS("-Wstack-protector")
    ENDIF()

    # Fix undefined reference to `__stack_chk_fail' error
    ADD_PLATFORM_LINKFLAGS("-lc")

    IF(NOT APPLE)
      ADD_PLATFORM_LINKFLAGS("-Wl,--no-undefined -Wl,--as-needed")

      IF(WITH_STATIC_RUNTIMES)
        ADD_PLATFORM_LINKFLAGS("-static-libstdc++")
      ENDIF()
    ENDIF()

    IF(NOT APPLE)
      # hardening
      ADD_PLATFORM_LINKFLAGS("-Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now")
    ENDIF()

    IF(WITH_SYMBOLS)
      SET(NL_RELEASE_CFLAGS "${NL_RELEASE_CFLAGS} -g")
    ELSE()
      IF(APPLE)
        SET(NL_RELEASE_LINKFLAGS "-Wl,-dead_strip ${NL_RELEASE_LINKFLAGS}")
      ELSE()
        SET(NL_RELEASE_LINKFLAGS "-Wl,-s ${NL_RELEASE_LINKFLAGS}")
      ENDIF()
    ENDIF()

    SET(NL_DEBUG_CFLAGS "-g -DNL_DEBUG -D_DEBUG ${NL_DEBUG_CFLAGS}")
    SET(NL_RELEASE_CFLAGS "-DNL_RELEASE -DNDEBUG -O3 ${NL_RELEASE_CFLAGS}")
  ENDIF()
ENDMACRO()

MACRO(NL_SETUP_BUILD_FLAGS)
  SET(CMAKE_C_FLAGS ${PLATFORM_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS ${PLATFORM_CXXFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS ${PLATFORM_LINKFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS ${PLATFORM_LINKFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS ${PLATFORM_LINKFLAGS} CACHE STRING "" FORCE)

  ## Debug
  SET(CMAKE_C_FLAGS_DEBUG ${NL_DEBUG_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS_DEBUG ${NL_DEBUG_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG ${NL_DEBUG_LINKFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG ${NL_DEBUG_LINKFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG ${NL_DEBUG_LINKFLAGS} CACHE STRING "" FORCE)

  ## Release
  SET(CMAKE_C_FLAGS_RELEASE ${NL_RELEASE_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS_RELEASE ${NL_RELEASE_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE ${NL_RELEASE_LINKFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE ${NL_RELEASE_LINKFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE ${NL_RELEASE_LINKFLAGS} CACHE STRING "" FORCE)
ENDMACRO(NL_SETUP_BUILD_FLAGS)

# Macro to create x_ABSOLUTE_PREFIX from x_PREFIX
MACRO(NL_MAKE_ABSOLUTE_PREFIX NAME_RELATIVE NAME_ABSOLUTE)
  IF(IS_ABSOLUTE "${${NAME_RELATIVE}}")
    SET(${NAME_ABSOLUTE} ${${NAME_RELATIVE}})
  ELSE()
    IF(WITH_UNIX_STRUCTURE)
      SET(${NAME_ABSOLUTE} ${CMAKE_INSTALL_PREFIX}/${${NAME_RELATIVE}})
    ELSE()
      SET(${NAME_ABSOLUTE} ${${NAME_RELATIVE}})
    ENDIF()
  ENDIF()
ENDMACRO(NL_MAKE_ABSOLUTE_PREFIX)

MACRO(NL_SETUP_PREFIX_PATHS)
  IF(TARGET_X64 AND WIN32)
    SET(LIB_SUFFIX "64")
  ELSE()
    SET(LIB_SUFFIX "")
  ENDIF()

  ## Allow override of install_prefix/etc path.
  IF(NOT NL_ETC_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(NL_ETC_PREFIX "etc/nel" CACHE PATH "Installation path for configurations")
    ELSE()
      SET(NL_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(NL_ETC_PREFIX NL_ETC_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT NL_SHARE_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(NL_SHARE_PREFIX "share/nel" CACHE PATH "Installation path for data.")
    ELSE()
      SET(NL_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(NL_SHARE_PREFIX NL_SHARE_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT NL_SBIN_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(NL_SBIN_PREFIX "sbin${LIB_SUFFIX}" CACHE PATH "Installation path for admin tools and services.")
    ELSE()
      SET(NL_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(NL_SBIN_PREFIX NL_SBIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT NL_BIN_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(NL_BIN_PREFIX "bin${LIB_SUFFIX}" CACHE PATH "Installation path for tools and applications.")
    ELSE()
      SET(NL_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(NL_BIN_PREFIX NL_BIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_LIB_PREFIX)
    IF(LIBRARY_ARCHITECTURE)
      SET(NL_LIB_PREFIX "lib/${LIBRARY_ARCHITECTURE}" CACHE PATH "Installation path for libraries.")
    ELSE()
      SET(NL_LIB_PREFIX "lib${LIB_SUFFIX}" CACHE PATH "Installation path for libraries.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(NL_LIB_PREFIX NL_LIB_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_DRIVER_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      IF(LIBRARY_ARCHITECTURE)
        SET(NL_DRIVER_PREFIX "lib/${LIBRARY_ARCHITECTURE}/nel" CACHE PATH "Installation path for drivers.")
      ELSE()
        IF(WIN32)
          SET(NL_DRIVER_PREFIX "bin${LIB_SUFFIX}" CACHE PATH "Installation path for drivers.")
        ELSE()
          SET(NL_DRIVER_PREFIX "lib${LIB_SUFFIX}/nel" CACHE PATH "Installation path for drivers.")
        ENDIF()
      ENDIF()
    ELSE()
      SET(NL_DRIVER_PREFIX "." CACHE PATH "Installation path for drivers.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(NL_DRIVER_PREFIX NL_DRIVER_ABSOLUTE_PREFIX)
ENDMACRO(NL_SETUP_PREFIX_PATHS)

MACRO(RYZOM_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT RYZOM_ETC_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(RYZOM_ETC_PREFIX "etc/ryzom" CACHE PATH "Installation path for configurations")
    ELSE()
      SET(RYZOM_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_ETC_PREFIX RYZOM_ETC_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT RYZOM_SHARE_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(RYZOM_SHARE_PREFIX "share/ryzom" CACHE PATH "Installation path for data.")
    ELSE()
      SET(RYZOM_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_SHARE_PREFIX RYZOM_SHARE_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT RYZOM_SBIN_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(RYZOM_SBIN_PREFIX "sbin${LIB_SUFFIX}" CACHE PATH "Installation path for admin tools and services.")
    ELSE()
      SET(RYZOM_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_SBIN_PREFIX RYZOM_SBIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT RYZOM_BIN_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(RYZOM_BIN_PREFIX "bin${LIB_SUFFIX}" CACHE PATH "Installation path for tools.")
    ELSE()
      SET(RYZOM_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_BIN_PREFIX RYZOM_BIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT RYZOM_LIB_PREFIX)
    IF(LIBRARY_ARCHITECTURE)
      SET(RYZOM_LIB_PREFIX "lib/${LIBRARY_ARCHITECTURE}" CACHE PATH "Installation path for libraries.")
    ELSE()
      SET(RYZOM_LIB_PREFIX "lib${LIB_SUFFIX}" CACHE PATH "Installation path for libraries.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_LIB_PREFIX RYZOM_LIB_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/games path.
  IF(NOT RYZOM_GAMES_PREFIX)
    IF(WITH_UNIX_STRUCTURE)
      SET(RYZOM_GAMES_PREFIX "games" CACHE PATH "Installation path for client.")
    ELSE()
      SET(RYZOM_GAMES_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ENDIF()
  ENDIF()
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_GAMES_PREFIX RYZOM_GAMES_ABSOLUTE_PREFIX)

ENDMACRO(RYZOM_SETUP_PREFIX_PATHS)

MACRO(SETUP_EXTERNAL)
  IF(WITH_EXTERNAL)
    FIND_PACKAGE(External REQUIRED)
  ENDIF()

  IF(WIN32)
    FIND_PACKAGE(External REQUIRED)

    # If using custom boost, we need to define the right variables used by official boost CMake module
    IF(DEFINED BOOST_DIR)
      SET(BOOST_INCLUDEDIR ${BOOST_DIR}/include)
      SET(BOOST_LIBRARYDIR ${BOOST_DIR}/lib)
    ENDIF()
  ELSE()
    FIND_PACKAGE(External QUIET)

    IF(APPLE)
      IF(WITH_STATIC_EXTERNAL)
        # Look only for static libraries because systems libraries are using Frameworks
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
      ELSE()
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .dylib .so .a)
      ENDIF()
    ELSE()
      IF(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a .so)
      ELSE()
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)
      ENDIF()
    ENDIF()
  ENDIF()

  # Android, iOS and Mac OS X have pthread, but no need to link to libpthread
  IF(ANDROID OR APPLE)
    SET(CMAKE_USE_PTHREADS_INIT 1)
    SET(Threads_FOUND TRUE)
  ELSE()
    SET(THREADS_HAVE_PTHREAD_ARG ON)
    FIND_PACKAGE(Threads)
    # TODO: replace all -l<lib> by absolute path to <lib> in CMAKE_THREAD_LIBS_INIT
  ENDIF()

  IF(MSVC)
    FIND_PACKAGE(MSVC REQUIRED)
    FIND_PACKAGE(WindowsSDK REQUIRED)
  ENDIF()
ENDMACRO()
