#-----------------------------------------------------------------------------
#Platform specifics

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common)

SET(RZ_SERVER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src)

ADD_SUBDIRECTORY(common)

IF(WITH_RYZOM_GAMESHARE)
  # add target alias
  SET(RYZOM_GAMESHARE_LIBRARIES ryzom_gameshare)
ENDIF()

ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(client)

IF(WITH_RYZOM_SERVER OR WITH_RYZOM_TOOLS)
  # Need servershare for build packed collision tool
  # Need aishare for build wmap tool
  FIND_PACKAGE(MySQL)
  IF(MYSQL_FOUND)
    ADD_SUBDIRECTORY(server)
  ENDIF()
ENDIF()
