#-----------------------------------------------------------------------------
#Platform specifics

FIND_PACKAGE(ZLIB)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common/src )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common )
SET(RZ_SERVER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/server/src)

ADD_SUBDIRECTORY(common)

IF(WITH_RYZOM_CLIENT)
  IF(NOT WITH_GUI)
    MESSAGE( FATAL_ERROR "The client cannot be built without the NeL GUI Library (WITH_GUI)")
  ENDIF(NOT WITH_GUI)

  ADD_SUBDIRECTORY(client)
ELSEIF(WITH_RYZOM_TOOLS)
  # Need clientsheets lib for sheets packer tool
  ADD_SUBDIRECTORY(client)
ENDIF(WITH_RYZOM_CLIENT)

IF(WITH_RYZOM_TOOLS)
  ADD_SUBDIRECTORY(tools)
ENDIF(WITH_RYZOM_TOOLS)

IF(WITH_RYZOM_SERVER)

  FIND_PACKAGE(MySQL REQUIRED)
  ADD_SUBDIRECTORY(server)
  
ELSEIF(WITH_RYZOM_TOOLS)

  # Need servershare for build packed collision tool
  # Need aishare for build wmap tool
  ADD_SUBDIRECTORY(server)

ENDIF(WITH_RYZOM_SERVER)
