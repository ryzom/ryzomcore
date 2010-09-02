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
    ADD_LIBRARY(${name} SHARED ${ARGN})
  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_TARGET_DRIVER)

###
# Helper macro that sets the default library properties.
# Argument: name - the target name whose properties are being set
# Argument:
###
MACRO(NL_DEFAULT_PROPS name label)
  GET_TARGET_PROPERTY(type ${name} TYPE)
  IF((${type} STREQUAL SHARED_LIBRARY) OR (${type} STREQUAL MODULE_LIBRARY))
    # Set versions only if target is a shared library or a module
    SET(versions VERSION ${NL_VERSION} SOVERSION ${NL_VERSION_MAJOR})
  ENDIF((${type} STREQUAL SHARED_LIBRARY) OR (${type} STREQUAL MODULE_LIBRARY))
  SET_TARGET_PROPERTIES(${name} PROPERTIES
    ${versions}
    PROJECT_LABEL ${label})
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
    SET_TARGET_PROPERTIES(${name} PROPERTIES
      LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
      LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}")
  ENDIF(WIN32)
  IF(WITH_STLPORT)
    TARGET_LINK_LIBRARIES(${name} ${STLPORT_LIBRARIES})
  ENDIF(WITH_STLPORT)
ENDMACRO(NL_ADD_RUNTIME_FLAGS)

MACRO(NL_ADD_STATIC_VID_DRIVERS name)
  IF(WITH_STATIC_DRIVERS)
    IF(WIN32)
	  IF(WITH_DRIVER_DIRECT3D)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_direct3d_win)
      ENDIF(WITH_DRIVER_DIRECT3D)
	
	  IF(WITH_DRIVER_DSOUND)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_dsound)
	  ENDIF(WITH_DRIVER_DSOUND)
	
	  IF(WITH_DRIVER_XAUDIO2)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_xaudio2)
	  ENDIF(WITH_DRIVER_XAUDIO2)
    ENDIF(WIN32)

    IF(WITH_DRIVER_OPENAL)
      TARGET_LINK_LIBRARIES(${name} nel_drv_openal)
    ENDIF(WITH_DRIVER_OPENAL)

    IF(WITH_DRIVER_FMOD)
      TARGET_LINK_LIBRARIES(${name} nel_drv_fmod)
    ENDIF(WITH_DRIVER_FMOD)

    IF(WITH_DRIVER_OPENGL)
      IF(WIN32)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_opengl_win)
	  ELSE(WIN32)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_opengl)
      ENDIF(WIN32)
    ENDIF(WITH_DRIVER_OPENGL)
  ENDIF(WITH_STATIC_DRIVERS)
ENDMACRO(NL_ADD_STATIC_VID_DRIVERS)

MACRO(NL_ADD_STATIC_SND_DRIVERS name)
  IF(WITH_STATIC_DRIVERS)
    IF(WIN32)
	  IF(WITH_DRIVER_DSOUND)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_dsound)
	  ENDIF(WITH_DRIVER_DSOUND)
	
	  IF(WITH_DRIVER_XAUDIO2)
	    TARGET_LINK_LIBRARIES(${name} nel_drv_xaudio2)
	  ENDIF(WITH_DRIVER_XAUDIO2)
    ENDIF(WIN32)

    IF(WITH_DRIVER_OPENAL)
      TARGET_LINK_LIBRARIES(${name} nel_drv_openal)
    ENDIF(WITH_DRIVER_OPENAL)

    IF(WITH_DRIVER_FMOD)
      TARGET_LINK_LIBRARIES(${name} nel_drv_fmod)
    ENDIF(WITH_DRIVER_FMOD)

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
  # Optional support
  ###
  OPTION(WITH_GTK         "With GTK Support"                                      OFF)
  OPTION(WITH_QT          "With QT Support"                                       OFF)
  OPTION(WITH_STLPORT     "With STLport support."                                 OFF)
  OPTION(BUILD_DASHBOARD  "Build to the CDash dashboard"                          OFF)

  OPTION(WITH_NEL	  "Build NeL (nearly always required)."						  ON )
  OPTION(WITH_NELNS	  "Build NeL Network Services."								  ON )
  OPTION(WITH_RYZOM	  "Build Ryzom Core."										  ON )
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
  OPTION(WITH_DRIVER_DIRECT3D     "Build Direct3D Driver (3D)"                    OFF)
  OPTION(WITH_DRIVER_OPENAL       "Build OpenAL Driver (Sound)"                   ON )
  OPTION(WITH_DRIVER_FMOD         "Build FMOD Driver (Sound)"                     OFF)
  OPTION(WITH_DRIVER_DSOUND       "Build DirectSound Driver (Sound)"              OFF)
  OPTION(WITH_DRIVER_XAUDIO2      "Build XAudio2 Driver (Sound)"                  OFF)

  ###
  # Optional support
  ###
  OPTION(WITH_NEL_CEGUI       "Build CEGUI Renderer"                                  OFF)
  OPTION(WITH_NEL_TOOLS       "Build NeL Tools"                                       ON )
  OPTION(WITH_NEL_MAXPLUGIN   "Build NeL 3dsMax Plugin"                               OFF)
  OPTION(WITH_NEL_SAMPLES     "Build NeL Samples"                                     ON )
  OPTION(WITH_NEL_TESTS       "Build NeL Unit Tests"                                  ON )

ENDMACRO(NL_SETUP_NEL_DEFAULT_OPTIONS)

MACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)
  ###
  # Core libraries
  ###
  OPTION(WITH_RYZOM_CLIENT           "Build Ryzom Core Client"                       ON )
  OPTION(WITH_RYZOM_TOOLS            "Build Ryzom Core Tools"                        ON )
  OPTION(WITH_RYZOM_SERVER           "Build Ryzom Core Services"                     ON )
  OPTION(WITH_RYZOM_SOUND            "Enable Ryzom Core Sound"                       ON )

  OPTION(WITH_LUA51                  "Build Ryzom Core using Lua51"                  ON )
ENDMACRO(NL_SETUP_RYZOM_DEFAULT_OPTIONS)

MACRO(NL_SETUP_BUILD)

  #-----------------------------------------------------------------------------
  # Setup the buildmode variables.
  #
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
    SET(NL_DEBUG_CFLAGS "/EHa /Ob1 /Zi")
    SET(NL_RELEASE_CFLAGS "/EHa /Zi /Ox /Ob2 /Oi /Ot /Oy /GT /GF /GS-")
    SET(NL_RELEASEDEBUG_CFLAGS "/EHa /DNL_RELEASE_DEBUG /Zi /Ob2 /GF")
    SET(NL_DEBUG_LINK_FLAGS "/NODEFAULTLIB:msvcrt")
    SET(NL_RELEASE_LINK_FLAGS "/OPT:REF /OPT:ICF")
    SET(NL_RELEASEDEBUG_LINK_FLAGS "/OPT:REF /OPT:ICF")
  ELSE(WIN32)
    SET(PLATFORM_CFLAGS "-ftemplate-depth-24 -D_REENTRANT -Wall -ansi -W -Wpointer-arith -Wsign-compare -Wno-deprecated-declarations -Wno-multichar -Wno-long-long -Wno-unused")
    IF(WITH_COVERAGE)
      SET(PLATFORM_CFLAGS "-fprofile-arcs -ftest-coverage ${PLATFORM_CFLAGS}")
    ENDIF(WITH_COVERAGE)
    SET(PLATFORM_LINKFLAGS "${CMAKE_THREAD_LIBS_INIT} -lc -lm -lstdc++")
    IF(NOT APPLE)
      SET(PLATFORM_LINKFLAGS "${PLATFORM_LINKFLAGS} -lrt")
    ENDIF(NOT APPLE)
    SET(NL_DEBUG_CFLAGS "-DNL_DEBUG -g")
    SET(NL_RELEASE_CFLAGS "-DNL_RELEASE -O6 -g")
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
  #SET(CMAKE_DEBUG_POSTFIX "_d")
  #SET(CMAKE_RELEASE_POSTFIX "_r")

  ## None
  #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NL_NONE_CFLAGS} ${PLATFORM_CFLAGS} ")
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NL_NONE_CFLAGS} ${PLATFORM_CFLAGS} ")

  ## Debug
  SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${NL_DEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${NL_DEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG} ${NL_DEBUG_LINK_FLAGS} ${PLATFORM_LINKFLAGS} ")

  ## Release
  SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${NL_RELEASE_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${NL_RELEASE_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE} ${NL_RELEASE_LINK_FLAGS} ${PLATFORM_LINKFLAGS} ")

  ## RelWithDebInfo
  SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_LINK_FLAGS_RELWITHDEBINFO "${CMAKE_LINK_FLAGS_RELWITHDEBINFO} ${NL_RELEASEDEBUG_LINK_FLAGS} ${PLATFORM_LINKFLAGS} ")

  ## MinSizeRel
  SET(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${NL_RELEASEDEBUG_CFLAGS} ${PLATFORM_CFLAGS} ")
  SET(CMAKE_LINK_FLAGS_MINSIZEREL "${CMAKE_LINK_FLAGS_MINSIZEREL} ${NL_RELEASEDEBUG_LINK_CFLAGS} ${PLATFORM_LINKFLAGS} ")
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

ENDMACRO(NL_SETUP_PREFIX_PATHS)
