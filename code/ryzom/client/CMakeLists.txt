# Need clientsheets lib for sheets packer tool
ADD_SUBDIRECTORY(src)

IF(WITH_RYZOM_CLIENT)
  #ADD_SUBDIRECTORY(data)
  #ADD_SUBDIRECTORY(patcher)

  IF(UNIX AND NOT APPLE)
    ADD_SUBDIRECTORY(unix)
  ENDIF()

  IF(RYZOM_ETC_PREFIX)
    CONFIGURE_FILE(client_default.cfg.in "${CMAKE_CURRENT_BINARY_DIR}/client_default.cfg")
    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/client_default.cfg" DESTINATION ${RYZOM_ETC_PREFIX})
  ELSE()
    INSTALL(FILES client_default.cfg DESTINATION etc/ryzom)
  ENDIF()
ENDIF()
