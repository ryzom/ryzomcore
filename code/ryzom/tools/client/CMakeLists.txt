IF(WITH_RYZOM_CLIENT)
  ADD_SUBDIRECTORY(client_patcher)

  IF(WITH_QT OR WITH_QT5)
    ADD_SUBDIRECTORY(client_config_qt)
  ENDIF()
ENDIF()

ADD_SUBDIRECTORY(r2_islands_textures)
