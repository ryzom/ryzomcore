
IF(WITH_LIGO)
  IF(WITH_3D)
    ADD_SUBDIRECTORY(unbuild_land)
    IF(WITH_GEORGES)
      ADD_SUBDIRECTORY(unbuild_flora)
    ENDIF()
  ENDIF()
ENDIF()
