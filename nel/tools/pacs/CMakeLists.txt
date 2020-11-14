IF(WITH_3D)
  ADD_SUBDIRECTORY(build_ig_boxes)

  IF(WITH_LIGO)
    ADD_SUBDIRECTORY(build_rbank)
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY(build_indoor_rbank)
