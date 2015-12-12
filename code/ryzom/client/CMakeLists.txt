# Need clientsheets lib for sheets packer tool
ADD_SUBDIRECTORY(src)

IF(WITH_RYZOM_CLIENT)
  #ADD_SUBDIRECTORY(data)
  #ADD_SUBDIRECTORY(patcher)

  IF(UNIX AND NOT APPLE)
    ADD_SUBDIRECTORY(unix)
  ENDIF()

  INSTALL(FILES client_default.cfg DESTINATION ${RYZOM_ETC_PREFIX})
ENDIF()
