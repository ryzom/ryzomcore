# Force Release configuration for compiler checks
SET(CMAKE_TRY_COMPILE_CONFIGURATION "Release")

# Force Release configuration by default
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

###
# Helper macro that generates .pc and installs it.
# Argument: name - the name of the .pc package, e.g. "nel-pacs.pc"
###
MACRO(NL_GEN_PC name)
  IF(NOT WIN32 AND WITH_INSTALL_LIBRARIES)
    CONFIGURE_FILE(${name}.in "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}" DESTINATION ${NL_LIB_PREFIX}/pkgconfig)
  ENDIF(NOT WIN32 AND WITH_INSTALL_LIBRARIES)
ENDMACRO(NL_GEN_PC)

###
# Helper macro that generates revision.h from revision.h.in
###
MACRO(NL_GEN_REVISION_H)
  IF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h.in)
    SET(TOOL_FOUND OFF)

    IF(EXISTS "${CMAKE_SOURCE_DIR}/../.svn/")
      FIND_PACKAGE(Subversion)

      IF(SUBVERSION_FOUND)
        SET(TOOL_FOUND ON)
      ENDIF(SUBVERSION_FOUND)
    ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/../.svn/")

    IF(EXISTS "${CMAKE_SOURCE_DIR}/../.hg/")
      FIND_PACKAGE(Mercurial)

      IF(MERCURIAL_FOUND)
        SET(TOOL_FOUND ON)
      ENDIF(MERCURIAL_FOUND)
    ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/../.hg/")

    # if already generated
    IF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h)
      # copy it
      MESSAGE(STATUS "Copying provided revision.h...")
      FILE(COPY ${CMAKE_SOURCE_DIR}/revision.h DESTINATION ${CMAKE_BINARY_DIR})
      SET(HAVE_REVISION_H ON)
    ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h)

    IF(TOOL_FOUND)
      # a custom target that is always built
      ADD_CUSTOM_TARGET(revision ALL
        COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DROOT_DIR=${CMAKE_SOURCE_DIR}/..
        -DCMAKE_MODULE_PATH=${CMAKE_SOURCE_DIR}/CMakeModules
        -P ${CMAKE_SOURCE_DIR}/CMakeModules/GetRevision.cmake)

      # revision.h is a generated file
      SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/revision.h
        PROPERTIES GENERATED TRUE
        HEADER_FILE_ONLY TRUE)
      SET(HAVE_REVISION_H ON)
    ENDIF(TOOL_FOUND)

    IF(HAVE_REVISION_H)
      INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
      ADD_DEFINITIONS(-DHAVE_REVISION_H)
    ENDIF(HAVE_REVISION_H)
  ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/revision.h.in)
ENDMACRO(NL_GEN_REVISION_H)

###
#
###
MACRO(NL_TARGET_LIB name)
  IF(WITH_STATIC)
    ADD_LIBRARY(${name} STATIC ${ARGN})
  ELSE(WITH_STATIC)
    ADD_LIBRARY(${name} SHARED ${ARGN})
  ENDIF(WITH_STATIC)
ENDMACRO(NL_TARGET_LIB)

###
#
###
MACRO(NL_TARGET_DRIVER name)
  IF(WITH_STATIC_DRIVERS)
    ADD_LIBRARY(${name} STATIC ${ARGN})
  ELSE(WITH_STATIC_DRIVERS)
    ADD_LIBRARY(${name} MODULE ${ARGN})
  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_TARGET_DRIVER)

###
# Helper macro that sets the default library properties.
# Argument: name - the target name whose properties are being set
# Argument:
###
MACRO(NL_DEFAULT_PROPS name label)
  IF(HAVE_REVISION_H)
    # explicitly say that the target depends on revision.h
    ADD_DEPENDENCIES(${name} revision)
  ENDIF(HAVE_REVISION_H)

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
    ENDIF(NL_LIB_PREFIX)
  ENDIF(${type} STREQUAL SHARED_LIBRARY)

  IF(${type} STREQUAL EXECUTABLE AND WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(${name} PROPERTIES
      VERSION ${NL_VERSION}
      SOVERSION ${NL_VERSION_MAJOR}
      COMPILE_FLAGS "/GA"
      LINK_FLAGS "/VERSION:${NL_VERSION_MAJOR}.${NL_VERSION_MINOR}")
  ENDIF(${type} STREQUAL EXECUTABLE AND WIN32 AND NOT MINGW)
ENDMACRO(NL_DEFAULT_PROPS)

###
# Adds the target suffix on Windows.
# Argument: name - the library's target name.
###
MACRO(NL_ADD_LIB_SUFFIX name)
  IF(WIN32)
    SET_TARGET_PROPERTIES(${name} PROPERTIES DEBUG_POSTFIX "_d" RELEASE_POSTFIX "_r")
  ENDIF(WIN32)
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
  ENDIF(WIN32)
  IF(WITH_STLPORT)
    TARGET_LINK_LIBRARIES(${name} ${STLPORT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  ENDIF(WITH_STLPORT)
ENDMACRO(NL_ADD_RUNTIME_FLAGS)

MACRO(NL_ADD_STATIC_VID_DRIVERS name)
  IF(WITH_STATIC_DRIVERS)
    IF(WIN32)
      IF(WITH_DRIVER_DIRECT3D)
        TARGET_LINK_LIBRARIES(${name} nel_drv_direct3d_win)
      ENDIF(WITH_DRIVER_DIRECT3D)
    ENDIF(WIN32)

    IF(WITH_DRIVER_OPENGL)
      IF(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengl_win)
      ELSE(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengl)
      ENDIF(WIN32)
    ENDIF(WITH_DRIVER_OPENGL)

    IF(WITH_DRIVER_OPENGLES)
      IF(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengles_win)
      ELSE(WIN32)
        TARGET_LINK_LIBRARIES(${name} nel_drv_opengles)
      ENDIF(WIN32)
    ENDIF(WITH_DRIVER_OPENGLES)
  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_ADD_STATIC_VID_DRIVERS)

MACRO(NL_ADD_STATIC_SND_DRIVERS name)
  IF(WITH_STATIC_DRIVERS)
    IF(WIN32)
      IF(WITH_DRIVER_DSOUND)
        TARGET_LINK_LIBRARIES(${name} nel_drv_dsound_win)
      ENDIF(WITH_DRIVER_DSOUND)

      IF(WITH_DRIVER_XAUDIO2)
        TARGET_LINK_LIBRARIES(${name} nel_drv_xaudio2_win)
      ENDIF(WITH_DRIVER_XAUDIO2)

      IF(WITH_DRIVER_OPENAL)
        TARGET_LINK_LIBRARIES(${name} nel_drv_openal_win)
      ENDIF(WITH_DRIVER_OPENAL)

      IF(WITH_DRIVER_FMOD)
        TARGET_LINK_LIBRARIES(${name} nel_drv_fmod_win)
      ENDIF(WITH_DRIVER_FMOD)
    ELSE(WIN32)
      IF(WITH_DRIVER_OPENAL)
        TARGET_LINK_LIBRARIES(${name} nel_drv_openal)
      ENDIF(WITH_DRIVER_OPENAL)

      IF(WITH_DRIVER_FMOD)
        TARGET_LINK_LIBRARIES(${name} nel_drv_fmod)
      ENDIF(WITH_DRIVER_FMOD)
    ENDIF(WIN32)

  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_ADD_STATIC_SND_DRIVERS)

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
  ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

ENDMACRO(CHECK_OUT_OF_SOURCE)

MACRO(NL_SETUP_DEFAULT_OPTIONS)
  ###
  # Features
  ###
  OPTION(WITH_LOGGING             "With Logging"                                  ON )
  OPTION(WITH_COVERAGE            "With Code Coverage Support"                    OFF)
  OPTION(WITH_PCH                 "With Precompiled Headers"                      ON )
  OPTION(FINAL_VERSION            "Build in Final Version mode"                   ON )

  # Default to static building on Windows.
  IF(WIN32)
    OPTION(WITH_STATIC            "With static libraries."                        ON )
  ELSE(WIN32)
    OPTION(WITH_STATIC            "With static libraries."                        OFF)
  ENDIF(WIN32)
  IF (WITH_STATIC)
    OPTION(WITH_STATIC_LIBXML2    "With static libxml2"                           ON )
  ELSE(WITH_STATIC)
    OPTION(WITH_STATIC_LIBXML2    "With static libxml2"                           OFF)
  ENDIF(WITH_STATIC)
  OPTION(WITH_STATIC_DRIVERS      "With static drivers."                          OFF)
  IF(WIN32)
    OPTION(WITH_EXTERNAL          "With provided external."                       ON )
  ELSE(WIN32)
    OPTION(WITH_EXTERNAL          "With provided external."                       OFF)
  ENDIF(WIN32)
  OPTION(WITH_STATIC_EXTERNAL     "With static external libraries"                OFF)
  OPTION(WITH_INSTALL_LIBRARIES   "Install development files."                    ON )

  ###
  # GUI toolkits
  ###
  OPTION(WITH_GTK                 "With GTK Support"                              OFF)
  OPTION(WITH_QT                  "With QT Support"                               OFF)

  IF(WIN32 AND MFC_FOUND)
    OPTION(WITH_MFC               "With MFC Support"                              ON )
  ELSE(WIN32 AND MFC_FOUND)
    OPTION(WITH_MFC               "With MFC Support"                              OFF)
  ENDIF(WIN32 AND MFC_FOUND)

  ###
  # Optional support
  ###
  OPTION(WITH_SYMBOLS             "Keep debug symbols in binaries"                OFF)

  IF(WIN32)
    OPTION(WITH_STLPORT           "With STLport support."                         ON )
  ELSE(WIN32)
    OPTION(WITH_STLPORT           "With STLport support."                         OFF)
  ENDIF(WIN32)

  OPTION(BUILD_DASHBOARD          "Build to the CDash dashboard"                  OFF)

  OPTION(WITH_NEL                 "Build NeL (nearly always required)."           ON )
  OPTION(WITH_NELNS               "Build NeL Network Services."                   OFF)
  OPTION(WITH_RYZOM               "Build Ryzom Core."                             ON )
  OPTION(WITH_SNOWBALLS           "Build Snowballs."                              OFF)
ENDMACRO(NL_SETUP_DEFAULT_OPTIONS)

MACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_NET                 "Build NLNET"                                   ON )
  OPTION(WITH_3D                  "Build NL3D"                                    ON )
  OPTION(WITH_GUI                 "Build GUI"                                     ON )
  OPTION(WITH_PACS                "Build NLPACS"                                  ON )
  OPTION(WITH_GEORGES             "Build NLGEORGES"                               ON )
  OPTION(WITH_LIGO                "Build NLLIGO"                                  ON )
  OPTION(WITH_LOGIC               "Build NLLOGIC"                                 ON )
  OPTION(WITH_SOUND               "Build NLSOUND"                                 ON )

  ###
  # Drivers Support
  ###
  OPTION(WITH_DRIVER_OPENGL       "Build OpenGL Driver (3D)"                      ON )
  OPTION(WITH_DRIVER_OPENGLES     "Build OpenGL ES Driver (3D)"                   OFF)
  OPTION(WITH_DRIVER_DIRECT3D     "Build Direct3D Driver (3D)"                    OFF)
  OPTION(WITH_DRIVER_OPENAL       "Build OpenAL Driver (Sound)"                   ON )
  OPTION(WITH_DRIVER_FMOD         "Build FMOD Driver (Sound)"                     OFF)
  OPTION(WITH_DRIVER_DSOUND       "Build DirectSound Driver (Sound)"              OFF)
  OPTION(WITH_DRIVER_XAUDIO2      "Build XAudio2 Driver (Sound)"                  OFF)

  ###
  # Optional support
  ###
  OPTION(WITH_NEL_CEGUI           "Build CEGUI Renderer"                          OFF)
  OPTION(WITH_NEL_TOOLS           "Build NeL Tools"                               ON )
  OPTION(WITH_NEL_MAXPLUGIN       "Build NeL 3dsMax Plugin"                       OFF)
  OPTION(WITH_NEL_SAMPLES         "Build NeL Samples"                             ON )
  OPTION(WITH_NEL_TESTS           "Build NeL Unit Tests"                          ON )

  OPTION(WITH_LIBOVR              "With LibOVR support"                           OFF)
  OPTION(WITH_LIBVR               "With LibVR support"                            OFF)
  OPTION(WITH_PERFHUD             "With NVIDIA PerfHUD support"                   OFF)
ENDMACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)

MACRO(NL_SETUP_NELNS_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_NELNS_SERVER        "Build NeLNS Services"                          ON )
  OPTION(WITH_NELNS_LOGIN_SYSTEM  "Build NeLNS Login System Tools"                ON )
ENDMACRO(NL_SETUP_NELNS_DEFAULT_OPTIONS)

MACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_RYZOM_CLIENT        "Build Ryzom Core Client"                       ON )
  OPTION(WITH_RYZOM_TOOLS         "Build Ryzom Core Tools"                        ON )
  OPTION(WITH_RYZOM_SERVER        "Build Ryzom Core Services"                     ON )
  OPTION(WITH_RYZOM_SOUND         "Enable Ryzom Core Sound"                       ON )
  OPTION(WITH_RYZOM_PATCH         "Enable Ryzom in-game patch support"            OFF)

  ###
  # Optional support
  ###
  OPTION(WITH_LUA51               "Build Ryzom Core using Lua 5.1"                ON )
  OPTION(WITH_LUA52               "Build Ryzom Core using Lua 5.2"                OFF)
ENDMACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)

MACRO(NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_SNOWBALLS_CLIENT    "Build Snowballs Client"                        ON )
  OPTION(WITH_SNOWBALLS_SERVER    "Build Snowballs Services"                      ON )
ENDMACRO(NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS)

MACRO(ADD_PLATFORM_FLAGS _FLAGS)
  SET(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} ${_FLAGS}")
  SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} ${_FLAGS}")
ENDMACRO(ADD_PLATFORM_FLAGS)

MACRO(NL_SETUP_BUILD)

  #-----------------------------------------------------------------------------
  # Setup the buildmode variables.
  #
  # None                  = NL_RELEASE
  # Debug                 = NL_DEBUG
  # Release               = NL_RELEASE

  SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

  IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(NL_BUILD_MODE "NL_DEBUG")
  ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
    IF(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
    ELSE(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
      # enforce release mode if it's neither Debug nor Release
      SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")
  ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

  SET(HOST_CPU ${CMAKE_HOST_SYSTEM_PROCESSOR})

  IF(HOST_CPU MATCHES "(amd|AMD)64")
    SET(HOST_CPU "x86_64")
  ELSEIF(HOST_CPU MATCHES "i.86")
    SET(HOST_CPU "x86")
  ENDIF(HOST_CPU MATCHES "(amd|AMD)64")
  
  # Determine target CPU

  # If not specified, use the same CPU as host
  IF(NOT TARGET_CPU)
    SET(TARGET_CPU ${CMAKE_SYSTEM_PROCESSOR})
  ENDIF(NOT TARGET_CPU)

  IF(TARGET_CPU MATCHES "(amd|AMD)64")
    SET(TARGET_CPU "x86_64")
  ELSEIF(TARGET_CPU MATCHES "i.86")
    SET(TARGET_CPU "x86")
  ENDIF(TARGET_CPU MATCHES "(amd|AMD)64")

  IF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    SET(CLANG ON)
    MESSAGE(STATUS "Using Clang compiler")
  ENDIF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")

  IF(CMAKE_GENERATOR MATCHES "Xcode")
    SET(XCODE ON)
    MESSAGE(STATUS "Generating Xcode project")
  ENDIF(CMAKE_GENERATOR MATCHES "Xcode")

  IF(CMAKE_GENERATOR MATCHES "NMake")
    SET(NMAKE ON)
    MESSAGE(STATUS "Generating NMake project")
  ENDIF(CMAKE_GENERATOR MATCHES "NMake")

  # If target and host CPU are the same
  IF("${HOST_CPU}" STREQUAL "${TARGET_CPU}" AND NOT CMAKE_CROSSCOMPILING)
    # x86-compatible CPU
    IF(HOST_CPU MATCHES "x86")
      IF(NOT CMAKE_SIZEOF_VOID_P)
        INCLUDE (CheckTypeSize)
        CHECK_TYPE_SIZE("void*"  CMAKE_SIZEOF_VOID_P)
      ENDIF(NOT CMAKE_SIZEOF_VOID_P)

      # Using 32 or 64 bits libraries
      IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(TARGET_CPU "x86_64")
      ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(TARGET_CPU "x86")
      ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ELSEIF(HOST_CPU MATCHES "arm")
      SET(TARGET_CPU "arm")
    ELSE(HOST_CPU MATCHES "x86")
      SET(TARGET_CPU "unknown")
      MESSAGE(STATUS "Unknown architecture: ${HOST_CPU}")
    ENDIF(HOST_CPU MATCHES "x86")
    # TODO: add checks for PPC
  ELSE("${HOST_CPU}" STREQUAL "${TARGET_CPU}" AND NOT CMAKE_CROSSCOMPILING)
    MESSAGE(STATUS "Compiling on ${HOST_CPU} for ${TARGET_CPU}")
  ENDIF("${HOST_CPU}" STREQUAL "${TARGET_CPU}" AND NOT CMAKE_CROSSCOMPILING)

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
    ENDIF(TARGET_CPU STREQUAL "x86_64")

    IF(TARGET_ARM)
      IF(TARGET_ARMV7S)
        ADD_PLATFORM_FLAGS("-DHAVE_ARMV7S")
      ENDIF(TARGET_ARMV7S)

      IF(TARGET_ARMV7)
        ADD_PLATFORM_FLAGS("-DHAVE_ARMV7")
      ENDIF(TARGET_ARMV7)

      IF(TARGET_ARMV6)
        ADD_PLATFORM_FLAGS("-HAVE_ARMV6")
      ENDIF(TARGET_ARMV6)

      ADD_PLATFORM_FLAGS("-DHAVE_ARM")
    ENDIF(TARGET_ARM)

    IF(TARGET_X86)
      ADD_PLATFORM_FLAGS("-DHAVE_X86")
    ENDIF(TARGET_X86)

    IF(TARGET_X64)
      ADD_PLATFORM_FLAGS("-DHAVE_X64 -DHAVE_X86_64")
    ENDIF(TARGET_X64)

    IF(TARGET_MIPS)
      ADD_PLATFORM_FLAGS("-DHAVE_MIPS")
    ENDIF(TARGET_MIPS)
  ENDIF(NOT CMAKE_OSX_ARCHITECTURES)

  # Fix library paths suffixes for Debian MultiArch
  IF(LIBRARY_ARCHITECTURE)
    SET(CMAKE_LIBRARY_PATH /lib/${LIBRARY_ARCHITECTURE} /usr/lib/${LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_PATH})
    IF(TARGET_X64)
      SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib64 /usr/lib64)
    ELSEIF(TARGET_X86)
      SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib32 /usr/lib32)
    ENDIF(TARGET_X64)
  ENDIF(LIBRARY_ARCHITECTURE)

  IF(APPLE AND NOT IOS)
    SET(CMAKE_INCLUDE_PATH /opt/local/include ${CMAKE_INCLUDE_PATH})
    SET(CMAKE_LIBRARY_PATH /opt/local/lib ${CMAKE_LIBRARY_PATH})
  ENDIF(APPLE AND NOT IOS)

  IF(WITH_LOGGING)
    ADD_PLATFORM_FLAGS("-DENABLE_LOGS")
  ENDIF(WITH_LOGGING)

  IF(MSVC)
    IF(MSVC_VERSION EQUAL "1700" AND NOT MSVC11)
      SET(MSVC11 ON)
    ENDIF(MSVC_VERSION EQUAL "1700" AND NOT MSVC11)

    # Ignore default include paths
    ADD_PLATFORM_FLAGS("/X")

    IF(MSVC11)
      ADD_PLATFORM_FLAGS("/Gy- /MP")
      # /Ox is working with VC++ 2010, but custom optimizations don't exist
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 /GF- ${DEBUG_CFLAGS}")
    ELSEIF(MSVC10)
      ADD_PLATFORM_FLAGS("/Gy- /MP")
      # /Ox is working with VC++ 2010, but custom optimizations don't exist
      SET(RELEASE_CFLAGS "/Ox /GF /GS- ${RELEASE_CFLAGS}")
      # without inlining it's unusable, use custom optimizations again
      SET(DEBUG_CFLAGS "/Od /Ob1 /GF- ${DEBUG_CFLAGS}")
    ELSEIF(MSVC90)
      ADD_PLATFORM_FLAGS("/Gy- /MP")
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
    ELSE(MSVC11)
      MESSAGE(FATAL_ERROR "Can't determine compiler version ${MSVC_VERSION}")
    ENDIF(MSVC11)

    ADD_PLATFORM_FLAGS("/D_CRT_SECURE_NO_DEPRECATE /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /DWIN32 /D_WINDOWS /Zm1000 /wd4250")

    IF(TARGET_X64)
      # Fix a bug with Intellisense
      ADD_PLATFORM_FLAGS("/D_WIN64")
      # Fix a compilation error for some big C++ files
      SET(RELEASE_CFLAGS "${RELEASE_CFLAGS} /bigobj")
    ELSE(TARGET_X64)
      # Allows 32 bits applications to use 3 GB of RAM
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} /LARGEADDRESSAWARE")
    ENDIF(TARGET_X64)

    # Exceptions are only set for C++
    SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} /EHa")

    IF(WITH_SYMBOLS)
      SET(NL_RELEASE_CFLAGS "/Zi ${NL_RELEASE_CFLAGS}")
      SET(NL_RELEASE_LINKFLAGS "/DEBUG ${NL_RELEASE_LINKFLAGS}")
    ELSE(WITH_SYMBOLS)
      SET(NL_RELEASE_LINKFLAGS "/RELEASE ${NL_RELEASE_LINKFLAGS}")
    ENDIF(WITH_SYMBOLS)

    SET(NL_DEBUG_CFLAGS "/Zi /MDd /RTC1 /D_DEBUG ${DEBUG_CFLAGS} ${NL_DEBUG_CFLAGS}")
    SET(NL_RELEASE_CFLAGS "/MD /DNDEBUG ${RELEASE_CFLAGS} ${NL_RELEASE_CFLAGS}")
    SET(NL_DEBUG_LINKFLAGS "/DEBUG /OPT:NOREF /OPT:NOICF /NODEFAULTLIB:msvcrt ${MSVC_INCREMENTAL_YES_FLAG} ${NL_DEBUG_LINKFLAGS}")
    SET(NL_RELEASE_LINKFLAGS "/OPT:REF /OPT:ICF /INCREMENTAL:NO ${NL_RELEASE_LINKFLAGS}")

    IF(WITH_WARNINGS)
      SET(DEBUG_CFLAGS "/W4 ${DEBUG_CFLAGS}")
    ELSE(WITH_WARNINGS)
      SET(DEBUG_CFLAGS "/W3 ${DEBUG_CFLAGS}")
    ENDIF(WITH_WARNINGS)
  ELSE(MSVC)
    IF(WIN32)
      ADD_PLATFORM_FLAGS("-DWIN32 -D_WIN32")

      IF(CLANG)
        ADD_PLATFORM_FLAGS("-nobuiltininc")
      ENDIF(CLANG)
    ENDIF(WIN32)

    IF(APPLE)
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
            ELSE(_ARCH STREQUAL "i386")
              SET(_ARCHS "${_ARCHS} unknwon(${_ARCH})")
            ENDIF(_ARCH STREQUAL "i386")
          ENDFOREACH(_ARCH)
          MESSAGE(STATUS "Compiling under Mac OS X for ${TARGETS_COUNT} architectures: ${_ARCHS}")
        ELSE(CMAKE_OSX_ARCHITECTURES)
          SET(TARGETS_COUNT 0)
        ENDIF(CMAKE_OSX_ARCHITECTURES)

        IF(TARGETS_COUNT EQUAL 1)
          IF(TARGET_ARM)
            IF(TARGET_ARMV7S)
              ADD_PLATFORM_FLAGS("-arch armv7s -DHAVE_ARMV7S")
            ENDIF(TARGET_ARMV7S)

            IF(TARGET_ARMV7)
              ADD_PLATFORM_FLAGS("-arch armv7 -DHAVE_ARMV7")
            ENDIF(TARGET_ARMV7)

            IF(TARGET_ARMV6)
              ADD_PLATFORM_FLAGS("-arch armv6 -DHAVE_ARMV6")
            ENDIF(TARGET_ARMV6)

            IF(TARGET_ARMV5)
              ADD_PLATFORM_FLAGS("-arch armv5 -DHAVE_ARMV5")
            ENDIF(TARGET_ARMV5)

            ADD_PLATFORM_FLAGS("-mthumb -DHAVE_ARM")
          ENDIF(TARGET_ARM)

          IF(TARGET_X64)
            ADD_PLATFORM_FLAGS("-arch x86_64 -DHAVE_X64 -DHAVE_X86_64 -DHAVE_X86")
          ELSEIF(TARGET_X86)
            ADD_PLATFORM_FLAGS("-arch i386 -DHAVE_X86")
          ENDIF(TARGET_X64)

          IF(TARGET_MIPS)
            ADD_PLATFORM_FLAGS("-arch mips -DHAVE_MIPS")
          ENDIF(TARGET_MIPS)
        ELSEIF(TARGETS_COUNT EQUAL 0)
          # Not using CMAKE_OSX_ARCHITECTURES, HAVE_XXX already defined before
          IF(TARGET_ARM)
            IF(TARGET_ARMV7S)
              ADD_PLATFORM_FLAGS("-arch armv7s")
            ENDIF(TARGET_ARMV7S)

            IF(TARGET_ARMV7)
              ADD_PLATFORM_FLAGS("-arch armv7")
            ENDIF(TARGET_ARMV7)

            IF(TARGET_ARMV6)
              ADD_PLATFORM_FLAGS("-arch armv6")
            ENDIF(TARGET_ARMV6)

            IF(TARGET_ARMV5)
              ADD_PLATFORM_FLAGS("-arch armv5")
            ENDIF(TARGET_ARMV5)

            ADD_PLATFORM_FLAGS("-mthumb")
          ENDIF(TARGET_ARM)

          IF(TARGET_X64)
            ADD_PLATFORM_FLAGS("-arch x86_64")
          ELSEIF(TARGET_X86)
            ADD_PLATFORM_FLAGS("-arch i386")
          ENDIF(TARGET_X64)

          IF(TARGET_MIPS)
            ADD_PLATFORM_FLAGS("-arch mips")
          ENDIF(TARGET_MIPS)
        ELSE(TARGETS_COUNT EQUAL 1)
          IF(TARGET_ARMV6)
            ADD_PLATFORM_FLAGS("-Xarch_armv6 -mthumb -Xarch_armv6 -DHAVE_ARM -Xarch_armv6 -DHAVE_ARMV6")
          ENDIF(TARGET_ARMV6)

          IF(TARGET_ARMV7)
            ADD_PLATFORM_FLAGS("-Xarch_armv7 -mthumb -Xarch_armv7 -DHAVE_ARM -Xarch_armv7 -DHAVE_ARMV7")
          ENDIF(TARGET_ARMV7)

          IF(TARGET_X86)
            ADD_PLATFORM_FLAGS("-Xarch_i386 -DHAVE_X86")
          ENDIF(TARGET_X86)

          IF(TARGET_X64)
            ADD_PLATFORM_FLAGS("-Xarch_x86_64 -DHAVE_X64 -Xarch_x86_64 -DHAVE_X86_64")
          ENDIF(TARGET_X64)

          IF(TARGET_MIPS)
            ADD_PLATFORM_FLAGS("-Xarch_mips -DHAVE_MIPS")
          ENDIF(TARGET_MIPS)
        ENDIF(TARGETS_COUNT EQUAL 1)

        IF(IOS)
          SET(CMAKE_OSX_SYSROOT "" CACHE PATH "" FORCE)

          IF(IOS_VERSION)
            PARSE_VERSION_STRING(${IOS_VERSION} IOS_VERSION_MAJOR IOS_VERSION_MINOR IOS_VERSION_PATCH)
            CONVERT_VERSION_NUMBER(${IOS_VERSION_MAJOR} ${IOS_VERSION_MINOR} ${IOS_VERSION_PATCH} IOS_VERSION_NUMBER)

            ADD_PLATFORM_FLAGS("-D__IPHONE_OS_VERSION_MIN_REQUIRED=${IOS_VERSION_NUMBER}")
          ENDIF(IOS_VERSION)

          IF(CMAKE_IOS_SYSROOT)
            IF(TARGET_ARMV7S)
              IF(TARGETS_COUNT GREATER 1)
                SET(XARCH "-Xarch_armv7s ")
              ENDIF(TARGETS_COUNT GREATER 1)

              ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SYSROOT}")
              ADD_PLATFORM_FLAGS("${XARCH}-miphoneos-version-min=${IOS_VERSION}")
              SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} ${XARCH}-Wl,-iphoneos_version_min,${IOS_VERSION}")
            ENDIF(TARGET_ARMV7S)

            IF(TARGET_ARMV7)
              IF(TARGETS_COUNT GREATER 1)
                SET(XARCH "-Xarch_armv7 ")
              ENDIF(TARGETS_COUNT GREATER 1)

              ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SYSROOT}")
              ADD_PLATFORM_FLAGS("${XARCH}-miphoneos-version-min=${IOS_VERSION}")
              SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} ${XARCH}-Wl,-iphoneos_version_min,${IOS_VERSION}")
            ENDIF(TARGET_ARMV7)

            IF(TARGET_ARMV6)
              IF(TARGETS_COUNT GREATER 1)
                SET(XARCH "-Xarch_armv6 ")
              ENDIF(TARGETS_COUNT GREATER 1)

              ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SYSROOT}")
              ADD_PLATFORM_FLAGS("${XARCH}-miphoneos-version-min=${IOS_VERSION}")
              SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} ${XARCH}-Wl,-iphoneos_version_min,${IOS_VERSION}")
            ENDIF(TARGET_ARMV6)
          ENDIF(CMAKE_IOS_SYSROOT)

          IF(CMAKE_IOS_SIMULATOR_SYSROOT AND TARGET_X86)
            IF(TARGETS_COUNT GREATER 1)
              SET(XARCH "-Xarch_i386 ")
            ENDIF(TARGETS_COUNT GREATER 1)

            ADD_PLATFORM_FLAGS("${XARCH}-isysroot${CMAKE_IOS_SIMULATOR_SYSROOT}")
            ADD_PLATFORM_FLAGS("${XARCH}-mios-simulator-version-min=${IOS_VERSION}")
            IF(CMAKE_OSX_DEPLOYMENT_TARGET)
              SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} ${XARCH}-Wl,-macosx_version_min,${CMAKE_OSX_DEPLOYMENT_TARGET}")
            ENDIF(CMAKE_OSX_DEPLOYMENT_TARGET)
          ENDIF(CMAKE_IOS_SIMULATOR_SYSROOT AND TARGET_X86)
        ELSE(IOS)
          # Always force -mmacosx-version-min to override environement variable
          IF(CMAKE_OSX_DEPLOYMENT_TARGET)
            SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,-macosx_version_min,${CMAKE_OSX_DEPLOYMENT_TARGET}")
          ENDIF(CMAKE_OSX_DEPLOYMENT_TARGET)
        ENDIF(IOS)

        SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,-headerpad_max_install_names")

        IF(HAVE_FLAG_SEARCH_PATHS_FIRST)
          SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,-search_paths_first")
        ENDIF(HAVE_FLAG_SEARCH_PATHS_FIRST)
      ENDIF(NOT XCODE)
    ELSE(APPLE)
      IF(HOST_CPU STREQUAL "x86_64" AND TARGET_CPU STREQUAL "x86")
        ADD_PLATFORM_FLAGS("-m32 -march=i686")
      ENDIF(HOST_CPU STREQUAL "x86_64" AND TARGET_CPU STREQUAL "x86")

      IF(HOST_CPU STREQUAL "x86" AND TARGET_CPU STREQUAL "x86_64")
        ADD_PLATFORM_FLAGS("-m64")
      ENDIF(HOST_CPU STREQUAL "x86" AND TARGET_CPU STREQUAL "x86_64")
    ENDIF(APPLE)

    ADD_PLATFORM_FLAGS("-D_REENTRANT -pipe -fno-strict-aliasing")

    IF(WITH_COVERAGE)
      ADD_PLATFORM_FLAGS("-fprofile-arcs -ftest-coverage")
    ENDIF(WITH_COVERAGE)

    IF(WITH_WARNINGS)
      ADD_PLATFORM_FLAGS("-Wall -W -Wpointer-arith -Wsign-compare -Wno-deprecated-declarations -Wno-multichar -Wno-unused")
      IF(CLANG)
        ADD_PLATFORM_FLAGS("-std=gnu99")
      ELSE(CLANG)
        ADD_PLATFORM_FLAGS("-ansi")
      ENDIF(CLANG)
    ENDIF(WITH_WARNINGS)

    IF(ANDROID)
      ADD_PLATFORM_FLAGS("--sysroot=${PLATFORM_ROOT}")
      ADD_PLATFORM_FLAGS("-ffunction-sections -funwind-tables")
      ADD_PLATFORM_FLAGS("-DANDROID")
      ADD_PLATFORM_FLAGS("-Wa,--noexecstack")

      IF(TARGET_ARM)
        ADD_PLATFORM_FLAGS("-fpic -fstack-protector")
        ADD_PLATFORM_FLAGS("-D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__")

        IF(TARGET_ARMV7)
          ADD_PLATFORM_FLAGS("-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16")
          SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -march=armv7-a -Wl,--fix-cortex-a8")
        ELSEIF(TARGET_ARMV5)
          ADD_PLATFORM_FLAGS("-march=armv5te -mtune=xscale -msoft-float")
        ENDIF(TARGET_ARMV7)

        SET(TARGET_THUMB ON)
        IF(TARGET_THUMB)
          ADD_PLATFORM_FLAGS("-mthumb -fno-strict-aliasing -finline-limit=64")
          SET(DEBUG_CFLAGS "${DEBUG_CFLAGS} -marm")
        ELSE(TARGET_THUMB)
          ADD_PLATFORM_FLAGS("-funswitch-loops -finline-limit=300")
          SET(DEBUG_CFLAGS "${DEBUG_CFLAGS} -fno-strict-aliasing")
          SET(RELEASE_CFLAGS "${RELEASE_CFLAGS} -fstrict-aliasing")
        ENDIF(TARGET_THUMB)
      ELSEIF(TARGET_X86)
        # Optimizations for Intel Atom
        ADD_PLATFORM_FLAGS("-march=i686 -mtune=atom -mstackrealign -msse3 -mfpmath=sse -m32 -flto -ffast-math -funroll-loops")
        ADD_PLATFORM_FLAGS("-fstack-protector -funswitch-loops -finline-limit=300")
        SET(RELEASE_CFLAGS "${RELEASE_CFLAGS} -fstrict-aliasing")
        SET(DEBUG_CFLAGS "${DEBUG_CFLAGS} -fno-strict-aliasing")
      ELSEIF(TARGET_MIPS)
        ADD_PLATFORM_FLAGS("-fpic -finline-functions -fmessage-length=0 -fno-inline-functions-called-once -fgcse-after-reload -frerun-cse-after-loop -frename-registers -fno-strict-aliasing")
        SET(RELEASE_CFLAGS "${RELEASE_CFLAGS} -funswitch-loops -finline-limit=300")
      ENDIF(TARGET_ARM)
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -L${PLATFORM_ROOT}/usr/lib")
    ENDIF(ANDROID)

    IF(APPLE)
      ADD_PLATFORM_FLAGS("-gdwarf-2")
    ENDIF(APPLE)

    # Fix "relocation R_X86_64_32 against.." error on x64 platforms
    IF(TARGET_X64 AND WITH_STATIC AND NOT WITH_STATIC_DRIVERS AND NOT MINGW)
      ADD_PLATFORM_FLAGS("-fPIC")
    ENDIF(TARGET_X64 AND WITH_STATIC AND NOT WITH_STATIC_DRIVERS AND NOT MINGW)

    SET(PLATFORM_CXXFLAGS "${PLATFORM_CXXFLAGS} -ftemplate-depth-48")

    IF(NOT APPLE)
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -Wl,--no-undefined -Wl,--as-needed")
    ENDIF(NOT APPLE)

    IF(WITH_SYMBOLS)
      SET(NL_RELEASE_CFLAGS "${NL_RELEASE_CFLAGS} -g")
    ELSE(WITH_SYMBOLS)
      IF(APPLE)
        SET(NL_RELEASE_LINKFLAGS "-Wl,-dead_strip ${NL_RELEASE_LINKFLAGS}")
      ELSE(APPLE)
        SET(NL_RELEASE_LINKFLAGS "-Wl,-s ${NL_RELEASE_LINKFLAGS}")
      ENDIF(APPLE)
    ENDIF(WITH_SYMBOLS)

    SET(NL_DEBUG_CFLAGS "-g -DNL_DEBUG -D_DEBUG ${NL_DEBUG_CFLAGS}")
    SET(NL_RELEASE_CFLAGS "-DNL_RELEASE -DNDEBUG -O3 ${NL_RELEASE_CFLAGS}")
  ENDIF(MSVC)
ENDMACRO(NL_SETUP_BUILD)

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
  ELSE(IS_ABSOLUTE "${${NAME_RELATIVE}}")
    IF(WIN32)
      SET(${NAME_ABSOLUTE} ${${NAME_RELATIVE}})
    ELSE(WIN32)
      SET(${NAME_ABSOLUTE} ${CMAKE_INSTALL_PREFIX}/${${NAME_RELATIVE}})
    ENDIF(WIN32)
  ENDIF(IS_ABSOLUTE "${${NAME_RELATIVE}}")
ENDMACRO(NL_MAKE_ABSOLUTE_PREFIX)

MACRO(NL_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT NL_ETC_PREFIX)
    IF(WIN32)
      SET(NL_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(NL_ETC_PREFIX "etc/nel" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT NL_ETC_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_ETC_PREFIX NL_ETC_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT NL_SHARE_PREFIX)
    IF(WIN32)
      SET(NL_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
    ELSE(WIN32)
      SET(NL_SHARE_PREFIX "share/nel" CACHE PATH "Installation path for data.")
    ENDIF(WIN32)
  ENDIF(NOT NL_SHARE_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_SHARE_PREFIX NL_SHARE_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT NL_SBIN_PREFIX)
    IF(WIN32)
      SET(NL_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
    ELSE(WIN32)
      SET(NL_SBIN_PREFIX "sbin" CACHE PATH "Installation path for admin tools and services.")
    ENDIF(WIN32)
  ENDIF(NOT NL_SBIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_SBIN_PREFIX NL_SBIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT NL_BIN_PREFIX)
    IF(WIN32)
      SET(NL_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(NL_BIN_PREFIX "bin" CACHE PATH "Installation path for tools and applications.")
    ENDIF(WIN32)
  ENDIF(NOT NL_BIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_BIN_PREFIX NL_BIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_LIB_PREFIX)
    IF(LIBRARY_ARCHITECTURE)
      SET(NL_LIB_PREFIX "lib/${LIBRARY_ARCHITECTURE}" CACHE PATH "Installation path for libraries.")
    ELSE(LIBRARY_ARCHITECTURE)
      SET(NL_LIB_PREFIX "lib" CACHE PATH "Installation path for libraries.")
    ENDIF(LIBRARY_ARCHITECTURE)
  ENDIF(NOT NL_LIB_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_LIB_PREFIX NL_LIB_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_DRIVER_PREFIX)
    IF(WIN32)
      SET(NL_DRIVER_PREFIX "." CACHE PATH "Installation path for drivers.")
    ELSE(WIN32)
      IF(LIBRARY_ARCHITECTURE)
        SET(NL_DRIVER_PREFIX "lib/${LIBRARY_ARCHITECTURE}/nel" CACHE PATH "Installation path for drivers.")
      ELSE(LIBRARY_ARCHITECTURE)
        SET(NL_DRIVER_PREFIX "lib/nel" CACHE PATH "Installation path for drivers.")
      ENDIF(LIBRARY_ARCHITECTURE)
    ENDIF(WIN32)
  ENDIF(NOT NL_DRIVER_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(NL_DRIVER_PREFIX NL_DRIVER_ABSOLUTE_PREFIX)

ENDMACRO(NL_SETUP_PREFIX_PATHS)

MACRO(RYZOM_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT RYZOM_ETC_PREFIX)
    IF(WIN32)
      SET(RYZOM_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(RYZOM_ETC_PREFIX "etc/ryzom" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_ETC_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_ETC_PREFIX RYZOM_ETC_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT RYZOM_SHARE_PREFIX)
    IF(WIN32)
      SET(RYZOM_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
    ELSE(WIN32)
      SET(RYZOM_SHARE_PREFIX "share/ryzom" CACHE PATH "Installation path for data.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_SHARE_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_SHARE_PREFIX RYZOM_SHARE_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT RYZOM_SBIN_PREFIX)
    IF(WIN32)
      SET(RYZOM_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
    ELSE(WIN32)
      SET(RYZOM_SBIN_PREFIX "sbin" CACHE PATH "Installation path for admin tools and services.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_SBIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_SBIN_PREFIX RYZOM_SBIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT RYZOM_BIN_PREFIX)
    IF(WIN32)
      SET(RYZOM_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(RYZOM_BIN_PREFIX "bin" CACHE PATH "Installation path for tools.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_BIN_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_BIN_PREFIX RYZOM_BIN_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT RYZOM_LIB_PREFIX)
    IF(LIBRARY_ARCHITECTURE)
      SET(RYZOM_LIB_PREFIX "lib/${LIBRARY_ARCHITECTURE}" CACHE PATH "Installation path for libraries.")
    ELSE(LIBRARY_ARCHITECTURE)
      SET(RYZOM_LIB_PREFIX "lib" CACHE PATH "Installation path for libraries.")
    ENDIF(LIBRARY_ARCHITECTURE)
  ENDIF(NOT RYZOM_LIB_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_LIB_PREFIX RYZOM_LIB_ABSOLUTE_PREFIX)

  ## Allow override of install_prefix/games path.
  IF(NOT RYZOM_GAMES_PREFIX)
    IF(WIN32)
      SET(RYZOM_GAMES_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(RYZOM_GAMES_PREFIX "games" CACHE PATH "Installation path for client.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_GAMES_PREFIX)
  NL_MAKE_ABSOLUTE_PREFIX(RYZOM_GAMES_PREFIX RYZOM_GAMES_ABSOLUTE_PREFIX)

ENDMACRO(RYZOM_SETUP_PREFIX_PATHS)

MACRO(SETUP_EXTERNAL)
  IF(WITH_EXTERNAL)
    FIND_PACKAGE(External REQUIRED)
  ENDIF(WITH_EXTERNAL)

  IF(WIN32)
    FIND_PACKAGE(External REQUIRED)

    # If using custom boost, we need to define the right variables used by official boost CMake module
    IF(DEFINED BOOST_DIR)
      SET(BOOST_INCLUDEDIR ${BOOST_DIR}/include)
      SET(BOOST_LIBRARYDIR ${BOOST_DIR}/lib)
    ENDIF(DEFINED BOOST_DIR)
  ELSE(WIN32)
    FIND_PACKAGE(External QUIET)

    IF(APPLE)
      IF(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
      ELSE(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .dylib .so .a)
      ENDIF(WITH_STATIC_EXTERNAL)
    ELSE(APPLE)
      IF(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .a .so)
      ELSE(WITH_STATIC_EXTERNAL)
        SET(CMAKE_FIND_LIBRARY_SUFFIXES .so .a)
      ENDIF(WITH_STATIC_EXTERNAL)
    ENDIF(APPLE)
  ENDIF(WIN32)

  # Android and iOS have pthread  
  IF(ANDROID OR IOS)
    SET(CMAKE_USE_PTHREADS_INIT 1)
    SET(Threads_FOUND TRUE)
  ELSE(ANDROID OR IOS)
    FIND_PACKAGE(Threads REQUIRED)
    # TODO: replace all -l<lib> by absolute path to <lib> in CMAKE_THREAD_LIBS_INIT
  ENDIF(ANDROID OR IOS)

  IF(WITH_STLPORT)
    FIND_PACKAGE(STLport REQUIRED)
    INCLUDE_DIRECTORIES(${STLPORT_INCLUDE_DIR})
  ENDIF(WITH_STLPORT)

  IF(MSVC)
    FIND_PACKAGE(MSVC REQUIRED)
    FIND_PACKAGE(WindowsSDK REQUIRED)
  ENDIF(MSVC)
ENDMACRO(SETUP_EXTERNAL)
