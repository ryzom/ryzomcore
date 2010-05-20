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
  # Build options
  ###

  OPTION(WITH_CLIENT		"Build Ryzom Core Client"			ON )
  OPTION(WITH_TOOLS		"Build Ryzom Core Tools"			ON )
  OPTION(WITH_SERVER		"Build Ryzom Core Services"			ON )
  OPTION(WITH_LUA51		"Build Ryzom Core using Lua51"			ON )
  OPTION(FINAL_VERSION		"Build in Final Version mode"			ON )

  ###
  # Features
  ###
  OPTION(WITH_LOGGING             "With Logging"                                  ON )
  OPTION(WITH_COVERAGE            "With Code Coverage Support"                    OFF)

  ###
  # Optional support
  ###
  OPTION(WITH_SOUND       "Build Sound Support"                                   OFF)
  OPTION(BUILD_DASHBOARD  "Build to the CDash dashboard"                          OFF)
ENDMACRO(NL_SETUP_DEFAULT_OPTIONS)


MACRO(NL_SETUP_BUILD)

  #-----------------------------------------------------------------------------
  # Setup the buildmode variables.
  #
  # None                  = NL_RELEASE_DEBUG
  # Debug                 = NL_DEBUG
  # Release               = NL_RELEASE
  # RelWithDebInfo        = NL_RELEASE_DEBUG
  # MinSizeRel            = NL_RELEASE_DEBUG

  # None                  = NL_RELEASE
  # Debug                 = NL_DEBUG
  # Release               = NL_RELEASE, NL_NO_DEBUG
  # RelWithDebInfo        = NL_RELEASE
  # MinSizeRel            = NL_RELEASE, NL_NO_DEBUG

  IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(NL_BUILD_MODE "NL_DEBUG")
  ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
    IF(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
    ELSE(CMAKE_BUILD_TYPE MATCHES "Release")
      SET(NL_BUILD_MODE "NL_RELEASE")
      # enforce release mode if it's neither Debug nor Release
      SET(CMAKE_BUILD_TYPE "Release")
    ENDIF(CMAKE_BUILD_TYPE MATCHES "Release")
  ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

  IF(WIN32)
    SET(NL_DEBUG_CFLAGS "/ZI /Gy /GS-")
    SET(NL_RELEASE_CFLAGS "/Ox /Ob2 /Oi /Ot /Oy /GT /GF")
    SET(NL_RELEASEDEBUG_CFLAGS "/DNL_RELEASE_DEBUG /Ob2 /GF")
  ELSE(WIN32)
    SET(PLATFORM_CFLAGS "-ftemplate-depth-48 -D_REENTRANT -Wall -ansi -W -Wpointer-arith -Wsign-compare -Wno-deprecated-declarations -Wno-multichar -Wno-long-long -Wno-unused -fno-strict-aliasing")
    IF(WITH_COVERAGE)
      SET(PLATFORM_CFLAGS "-fprofile-arcs -ftest-coverage ${PLATFORM_CFLAGS}")
    ENDIF(WITH_COVERAGE)
    SET(PLATFORM_LINKFLAGS "${CMAKE_THREAD_LIBS_INIT} -lc -lm -lstdc++ -lrt")
    SET(NL_DEBUG_CFLAGS "-DNL_DEBUG -g")
    SET(NL_RELEASE_CFLAGS "-DNL_RELEASE -O6")
    SET(NL_RELEASEDEBUG_CFLAGS "-DNL_RELEASE_DEBUG -g -finline-functions -O3 ")
    SET(NL_NONE_CFLAGS "-DNL_RELEASE -g -finline-functions -O2 ")
  ENDIF(WIN32)

  # Determine host CPU
  IF(UNIX AND NOT WIN32)
    FIND_PROGRAM(CMAKE_UNAME uname /bin /usr/bin /usr/local/bin )
    IF(CMAKE_UNAME)
      EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
      SET(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE INTERNAL "processor type (i386 and x86_64)")
      IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        ADD_DEFINITIONS(-DHAVE_X86_64)
      ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "ia64")
        ADD_DEFINITIONS(-DHAVE_IA64)
      ELSE(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        ADD_DEFINITIONS(-DHAVE_X86)
      ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    ELSE(CMAKE_UNAME)  # Assume that if uname is not found that we're x86.
      ADD_DEFINITIONS(-DHAVE_X86)
    ENDIF(CMAKE_UNAME)
  ENDIF(UNIX AND NOT WIN32)

ENDMACRO(NL_SETUP_BUILD)

MACRO(NL_SETUP_BUILD_FLAGS)
  ## None
  #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NL_NONE_CFLAGS} ${PLATFORM_CFLAGS} ")
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NL_NONE_CFLAGS} ${PLATFORM_CFLAGS} ")

  ## Debug
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${NL_DEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${NL_DEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")

  ## Release
  SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${NL_RELEASE_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${NL_RELEASE_CFLAGS} ${PLATFORM_CFLAGS} ")

  ## RelWithDebInfo
  SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")

  ## MinSizeRel
  SET(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
ENDMACRO(NL_SETUP_BUILD_FLAGS)

MACRO(NL_SETUP_PREFIX_PATHS)
  ## Allow override of install_prefix/etc path.
  IF(NOT NL_ETC_PREFIX)
    IF(WIN32)
      SET(NL_ETC_PREFIX "../etc" CACHE PATH "Installation path for configurations")
    ELSE(WIN32)
      SET(NL_ETC_PREFIX "${CMAKE_INSTALL_PREFIX}/etc" CACHE PATH "Installation path for configurations")
    ENDIF(WIN32)
  ENDIF(NOT NL_ETC_PREFIX)

  ## Allow override of install_prefix/share path.
  IF(NOT NL_SHARE_PREFIX)
    IF(WIN32)
	  SET(NL_SHARE_PREFIX "../share" CACHE PATH "Installation path for data.")
	ELSE(WIN32)
	  SET(NL_SHARE_PREFIX "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH "Installation path for data.")
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

  ## Allow override of install_prefix/bin path.
  IF(NOT NL_LOG_PREFIX)
    IF(WIN32)
      SET(NL_LOG_PREFIX "../var/log" CACHE PATH "Installation path for tools and applications.")
    ELSE(WIN32)
      SET(NL_LOG_PREFIX "${CMAKE_INSTALL_PREFIX}/var/log" CACHE PATH "Installation path for tools and applications.")
    ENDIF(WIN32)
  ENDIF(NOT NL_LOG_PREFIX)

ENDMACRO(NL_SETUP_PREFIX_PATHS)
