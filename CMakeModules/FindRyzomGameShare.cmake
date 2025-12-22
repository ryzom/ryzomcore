INCLUDE(FindHelpers)

FIND_PACKAGE_HELPER(RyzomGameShare game_share/continent.h RELEASE ryzom_gameshare_r ryzom_gameshare DEBUG ryzom_gameshare_d DIR ${NEL_DIR} ${RYZOM_DIR} SUFFIXES ryzom)

IF(RYZOMGAMESHARE_FOUND)
  FIND_PACKAGE(NeL REQUIRED)

  # TODO: ryzom_gameshare should probably publish its own config file
  IF(NOT TARGET ryzom_gameshare)
    ADD_LIBRARY(ryzom_gameshare UNKNOWN IMPORTED)
    SET_TARGET_PROPERTIES(ryzom_gameshare PROPERTIES
      IMPORTED_LOCATION ${RYZOMGAMESHARE_LIBRARIES}
      INTERFACE_INCLUDE_DIRECTORIES ${RYZOMGAMESHARE_INCLUDE_DIRS}
      INTERFACE_LINK_LIBRARIES "NeL::misc;NeL::net;NeL::ligo;NeL::georges"
    )
  ENDIF()
ENDIF()
