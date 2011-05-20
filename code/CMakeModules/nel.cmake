###
# Helper macro that generates .pc and installs it.
# Argument: name - the name of the .pc package, e.g. "nel-pacs.pc"
###
MACRO(NL_GEN_PC name)
  IF(NOT WIN32)
    CONFIGURE_FILE(${name}.in "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/${name}" DESTINATION lib/pkgconfig)
  ENDIF(NOT WIN32)
ENDMACRO(NL_GEN_PC)

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
  IF(NOT MSVC10)
    SET_TARGET_PROPERTIES(${name} PROPERTIES PROJECT_LABEL ${label})
  ENDIF(NOT MSVC10)
  GET_TARGET_PROPERTY(type ${name} TYPE)
  IF(${type} STREQUAL SHARED_LIBRARY)
    # Set versions only if target is a shared library
    SET_TARGET_PROPERTIES(${name} PROPERTIES
      VERSION ${NL_VERSION} SOVERSION ${NL_VERSION_MAJOR})
    IF(NL_LIB_PREFIX)
      SET_TARGET_PROPERTIES(${name} PROPERTIES INSTALL_NAME_DIR ${NL_LIB_PREFIX})
    ENDIF(NL_LIB_PREFIX)
  ENDIF(${type} STREQUAL SHARED_LIBRARY)
  IF(WITH_STLPORT AND WIN32)
    SET_TARGET_PROPERTIES(${name} PROPERTIES COMPILE_FLAGS "/X")
  ENDIF(WITH_STLPORT AND WIN32)
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
   cmake -G \"Unix Makefiles\" ..
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
  OPTION(WITH_STATIC_DRIVERS      "With static drivers."                          OFF)

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
  IF(WIN32)
    OPTION(WITH_STLPORT           "With STLport support."                         ON )
  ELSE(WIN32)
    OPTION(WITH_STLPORT           "With STLport support."                         OFF)
  ENDIF(WIN32)

  OPTION(BUILD_DASHBOARD          "Build to the CDash dashboard"                  OFF)

  OPTION(WITH_NEL	              "Build NeL (nearly always required)."           ON )
  OPTION(WITH_NELNS	              "Build NeL Network Services."                   OFF)
  OPTION(WITH_RYZOM	              "Build Ryzom Core."                             ON )
  OPTION(WITH_SNOWBALLS	          "Build Snowballs."                              OFF)
ENDMACRO(NL_SETUP_DEFAULT_OPTIONS)

MACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_NET                 "Build NLNET"                                   ON )
  OPTION(WITH_3D                  "Build NL3D"                                    ON )
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

  ###
  # Optional support
  ###
  OPTION(WITH_LUA51               "Build Ryzom Core using Lua51"                  ON )
ENDMACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)

MACRO(NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_SNOWBALLS_CLIENT    "Build Snowballs Client"                        ON )
  OPTION(WITH_SNOWBALLS_SERVER    "Build Snowballs Services"                      ON )
ENDMACRO(NL_SETUP_SNOWBALLS_DEFAULT_OPTIONS)

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

  IF(WIN32)
    IF(MSVC10)
      # /Ox is working with VC++ 2010, but custom optimizations don't exist
      SET(SPEED_OPTIMIZATIONS "/Ox /GF /GS-")
      # without inlining it's unusable, use custom optimizations again
      SET(MIN_OPTIMIZATIONS "/Od /Ob1")
	ELSE(MSVC10)
      # don't use a /O[012x] flag if you want custom optimizations
      SET(SPEED_OPTIMIZATIONS "/Ob2 /Oi /Ot /Oy /GT /GF /GS-")
      # without inlining it's unusable, use custom optimizations again
      SET(MIN_OPTIMIZATIONS "/Ob1")
    ENDIF(MSVC10)

    SET(PLATFORM_CFLAGS "/D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_WINDOWS /W3 /Zi /Zm1000 /MP")

    # Exceptions are only set for C++
    SET(PLATFORM_CXXFLAGS "${PLATFORM_CFLAGS} /EHa")

    # Common link flags
    SET(PLATFORM_LINKFLAGS "-DEBUG")

    SET(NL_DEBUG_CFLAGS "/MDd /RTC1 /D_DEBUG ${MIN_OPTIMIZATIONS}")
    SET(NL_RELEASE_CFLAGS "/MD /D NDEBUG ${SPEED_OPTIMIZATIONS}")
    SET(NL_DEBUG_LINKFLAGS "/NODEFAULTLIB:msvcrt /INCREMENTAL:YES")
    SET(NL_RELEASE_LINKFLAGS "/OPT:REF /OPT:ICF /INCREMENTAL:NO")
  ELSE(WIN32)
    SET(PLATFORM_CFLAGS "-g -pipe -ftemplate-depth-48 -D_REENTRANT -Wall -ansi -W -Wpointer-arith -Wsign-compare -Wno-deprecated-declarations -Wno-multichar -Wno-unused -fno-strict-aliasing")
    IF(WITH_COVERAGE)
      SET(PLATFORM_CFLAGS "-fprofile-arcs -ftest-coverage ${PLATFORM_CFLAGS}")
    ENDIF(WITH_COVERAGE)

    IF(APPLE)
      SET(PLATFORM_CFLAGS "-gdwarf-2 ${PLATFORM_CFLAGS}")
    ENDIF(APPLE)

    SET(PLATFORM_CXXFLAGS ${PLATFORM_CFLAGS})

    IF(NOT APPLE)
      SET(PLATFORM_LINKFLAGS "-Wl,--no-undefined -Wl,--as-needed")
    ENDIF(NOT APPLE)

    SET(NL_DEBUG_CFLAGS "-DNL_DEBUG -D_DEBUG")
    SET(NL_RELEASE_CFLAGS "-DNL_RELEASE -DNDEBUG -O6")

  ENDIF(WIN32)

  # Determine target CPU
#  IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")
    IF(NOT CMAKE_SIZEOF_VOID_P)
      INCLUDE (CheckTypeSize)
      CHECK_TYPE_SIZE("void*"  CMAKE_SIZEOF_VOID_P)
    ENDIF(NOT CMAKE_SIZEOF_VOID_P)

    # Using 32 or 64 bits libraries
    SET(TARGET_X86 1)
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
      SET(ARCH "x86_64")
      SET(TARGET_X64 1)
      ADD_DEFINITIONS(-DHAVE_X86_64)
    ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
      SET(ARCH "x86")
      ADD_DEFINITIONS(-DHAVE_X86)
    ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
#     ADD_DEFINITIONS(-DHAVE_IA64)
#  ENDIF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86")

ENDMACRO(NL_SETUP_BUILD)

MACRO(NL_SETUP_BUILD_FLAGS)
  SET(CMAKE_C_FLAGS ${PLATFORM_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS ${PLATFORM_CXXFLAGS} CACHE STRING "" FORCE)

  ## Debug
  SET(CMAKE_C_FLAGS_DEBUG ${NL_DEBUG_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS_DEBUG ${NL_DEBUG_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${PLATFORM_LINKFLAGS} ${NL_DEBUG_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${PLATFORM_LINKFLAGS} ${NL_DEBUG_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${PLATFORM_LINKFLAGS} ${NL_DEBUG_LINKFLAGS}" CACHE STRING "" FORCE)

  ## Release
  SET(CMAKE_C_FLAGS_RELEASE ${NL_RELEASE_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_CXX_FLAGS_RELEASE ${NL_RELEASE_CFLAGS} CACHE STRING "" FORCE)
  SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${NL_RELEASE_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${NL_RELEASE_LINKFLAGS}" CACHE STRING "" FORCE)
  SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${PLATFORM_LINKFLAGS} ${NL_RELEASE_LINKFLAGS}" CACHE STRING "" FORCE)
ENDMACRO(NL_SETUP_BUILD_FLAGS)

MACRO(NL_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT NL_ETC_PREFIX)
    IF(WIN32)
      SET(NL_ETC_PREFIX "../etc/nel" CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(NL_ETC_PREFIX "${CMAKE_INSTALL_PREFIX}/etc/nel" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT NL_ETC_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT NL_SHARE_PREFIX)
    IF(WIN32)
      SET(NL_SHARE_PREFIX "../share/nel" CACHE PATH "Installation path for data.")
    ELSE(WIN32)
      SET(NL_SHARE_PREFIX "${CMAKE_INSTALL_PREFIX}/share/nel" CACHE PATH "Installation path for data.")
    ENDIF(WIN32)
  ENDIF(NOT NL_SHARE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT NL_SBIN_PREFIX)
    IF(WIN32)
      SET(NL_SBIN_PREFIX "../sbin" CACHE PATH "Installation path for admin tools and services.")
    ELSE(WIN32)
      SET(NL_SBIN_PREFIX "${CMAKE_INSTALL_PREFIX}/sbin" CACHE PATH "Installation path for admin tools and services.")
    ENDIF(WIN32)
  ENDIF(NOT NL_SBIN_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT NL_BIN_PREFIX)
    IF(WIN32)
      SET(NL_BIN_PREFIX "../bin" CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(NL_BIN_PREFIX "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation path for tools and applications.")
    ENDIF(WIN32)
  ENDIF(NOT NL_BIN_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_LIB_PREFIX)
    IF(WIN32)
      SET(NL_LIB_PREFIX "../lib" CACHE PATH "Installation path for libraries.")
    ELSE(WIN32)
      SET(NL_LIB_PREFIX "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation path for libraries.")
    ENDIF(WIN32)
  ENDIF(NOT NL_LIB_PREFIX)

  ## Allow override of install_prefix/lib path.
  IF(NOT NL_DRIVER_PREFIX)
    IF(WIN32)
      SET(NL_DRIVER_PREFIX "../lib" CACHE PATH "Installation path for drivers.")
    ELSE(WIN32)
      SET(NL_DRIVER_PREFIX "${CMAKE_INSTALL_PREFIX}/lib/nel" CACHE PATH "Installation path for drivers.")
    ENDIF(WIN32)
  ENDIF(NOT NL_DRIVER_PREFIX)

ENDMACRO(NL_SETUP_PREFIX_PATHS)

MACRO(RYZOM_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix path.
  IF(NOT RYZOM_PREFIX)
    IF(WIN32)
      SET(RYZOM_PREFIX "." CACHE PATH "Installation path")
    ELSE(WIN32)
      SET(RYZOM_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE PATH "Installation path")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_PREFIX)

  ## Allow override of install_prefix/etc path.
  IF(NOT RYZOM_ETC_PREFIX)
    IF(WIN32)
      SET(RYZOM_ETC_PREFIX "." CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(RYZOM_ETC_PREFIX "${CMAKE_INSTALL_PREFIX}/etc/ryzom" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_ETC_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT RYZOM_SHARE_PREFIX)
    IF(WIN32)
	  SET(RYZOM_SHARE_PREFIX "." CACHE PATH "Installation path for data.")
	ELSE(WIN32)
	  SET(RYZOM_SHARE_PREFIX "${CMAKE_INSTALL_PREFIX}/share/ryzom" CACHE PATH "Installation path for data.")
	ENDIF(WIN32)
  ENDIF(NOT RYZOM_SHARE_PREFIX)

  ## Allow override of install_prefix/sbin path.
  IF(NOT RYZOM_SBIN_PREFIX)
	IF(WIN32)
	  SET(RYZOM_SBIN_PREFIX "." CACHE PATH "Installation path for admin tools and services.")
	ELSE(WIN32)
	  SET(RYZOM_SBIN_PREFIX "${CMAKE_INSTALL_PREFIX}/sbin" CACHE PATH "Installation path for admin tools and services.")
	ENDIF(WIN32)
  ENDIF(NOT RYZOM_SBIN_PREFIX)

  ## Allow override of install_prefix/bin path.
  IF(NOT RYZOM_BIN_PREFIX)
    IF(WIN32)
		SET(RYZOM_BIN_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
		SET(RYZOM_BIN_PREFIX "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation path for tools.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_BIN_PREFIX)

  ## Allow override of install_prefix/games path.
  IF(NOT RYZOM_GAMES_PREFIX)
    IF(WIN32)
		SET(RYZOM_GAMES_PREFIX "." CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
		SET(RYZOM_GAMES_PREFIX "${CMAKE_INSTALL_PREFIX}/games" CACHE PATH "Installation path for client.")
    ENDIF(WIN32)
  ENDIF(NOT RYZOM_GAMES_PREFIX)

ENDMACRO(RYZOM_SETUP_PREFIX_PATHS)
