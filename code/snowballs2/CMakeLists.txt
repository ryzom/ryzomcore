IF(WIN32)
  SET(SNOWBALLS_CONFIG_FILE "." CACHE FILEPATH "Snowballs config file location")
ELSE(WIN32)
  SET(SNOWBALLS_CONFIG_FILE "${NL_ETC_PREFIX}/snowballs" CACHE FILEPATH "Snowballs config file location")
ENDIF(WIN32)
SET(SNOWBALLS_DATA_FILE "${NL_SHARE_PREFIX}/games/snowballs" CACHE FILEPATH "Snowballs data file location")
SET(SNOWBALLS_LOG_FILE "${NL_LOG_PREFIX}/snowballs" CACHE FILEPATH "Snowballs log file location")

# Note: Not sure if this is still the best way to do this...
#INSTALL(CODE "FILE(MAKE_DIRECTORY ${SNOWBALLS_LOG_FILE})")

#-----------------------------------------------------------------------------
#Platform specifics

###
# An example of finding NeL 3D and CEGUI Renderer.
###
ADD_DEFINITIONS(-DSNOWBALLS_CONFIG="${SNOWBALLS_CONFIG_FILE}/" -DSNOWBALLS_STATE="${SNOWBALLS_LOG_FILE}/" -DSNOWBALLS_LOGS="${SNOWBALLS_LOG_FILE}/")

IF(WITH_SNOWBALLS_CLIENT)
  ADD_SUBDIRECTORY(client)
ENDIF(WITH_SNOWBALLS_CLIENT)

IF(WITH_SNOWBALLS_SERVER)
  ADD_SUBDIRECTORY(server)
ENDIF(WITH_SNOWBALLS_SERVER)

IF(WITH_SNOWBALLS_PACKAGE)
  INCLUDE(CMakePackaging.txt)
ENDIF(WITH_SNOWBALLS_PACKAGE)

