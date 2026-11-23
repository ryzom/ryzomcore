cmake_minimum_required(VERSION 3.28..4.1.1)

project(Snowballs VERSION 2.0.0 LANGUAGES CXX)

# the client and the collision service link 3d and pacs, the client sound when WITH_SOUND is on
set(SNOWBALLS_NEL_COMPONENTS 3d misc net pacs)
if(WITH_SNOWBALLS_CLIENT AND WITH_SOUND)
  list(APPEND SNOWBALLS_NEL_COMPONENTS sound)
endif()
find_package(NeL ${RyzomCore_VERSION} REQUIRED COMPONENTS ${SNOWBALLS_NEL_COMPONENTS})

IF(WIN32)
  SET(SNOWBALLS_CONFIG_FILE "." CACHE FILEPATH "Snowballs config file location")
ELSE()
  SET(SNOWBALLS_CONFIG_FILE "${NL_ETC_PREFIX}/snowballs" CACHE FILEPATH "Snowballs config file location")
ENDIF()
SET(SNOWBALLS_DATA_FILE "${NL_SHARE_PREFIX}/games/snowballs" CACHE FILEPATH "Snowballs data file location")

# The config and data files are installed to the two locations above, relative to the install prefix unless they are
# absolute. The programs look for them at the same locations made absolute, as the NL_*_ABSOLUTE_PREFIX paths are.
NL_MAKE_ABSOLUTE_PREFIX(SNOWBALLS_CONFIG_FILE SNOWBALLS_CONFIG_ABSOLUTE_FILE)
NL_MAKE_ABSOLUTE_PREFIX(SNOWBALLS_DATA_FILE SNOWBALLS_DATA_ABSOLUTE_FILE)

# Empty, the services log in their working directory, as they do without the -L option
SET(SNOWBALLS_LOG_FILE "" CACHE FILEPATH "Snowballs log file location (empty for the working directory)")
# "/snowballs" is what the former default, made from an NL_LOG_PREFIX that no longer exists, left in existing caches
IF(SNOWBALLS_LOG_FILE STREQUAL "/snowballs")
  SET(SNOWBALLS_LOG_FILE "" CACHE FILEPATH "Snowballs log file location (empty for the working directory)" FORCE)
ENDIF()
IF(SNOWBALLS_LOG_FILE)
  SET(SNOWBALLS_LOG_DIRECTORY "${SNOWBALLS_LOG_FILE}/")
ELSE()
  SET(SNOWBALLS_LOG_DIRECTORY "")
ENDIF()

# Note: Not sure if this is still the best way to do this...
#INSTALL(CODE "FILE(MAKE_DIRECTORY ${SNOWBALLS_LOG_FILE})")

#-----------------------------------------------------------------------------
#Platform specifics

###
# An example of finding NeL 3D and CEGUI Renderer.
###
add_compile_definitions(SNOWBALLS_CONFIG="${SNOWBALLS_CONFIG_ABSOLUTE_FILE}/" SNOWBALLS_STATE="${SNOWBALLS_LOG_DIRECTORY}" SNOWBALLS_LOGS="${SNOWBALLS_LOG_DIRECTORY}")

IF(WITH_SNOWBALLS_CLIENT)
  ADD_SUBDIRECTORY(client)
ENDIF()

IF(WITH_SNOWBALLS_SERVER)
  ADD_SUBDIRECTORY(server)
ENDIF()

IF(WITH_SNOWBALLS_PACKAGE)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)
  INCLUDE(CMakePackaging.txt)
ENDIF()

