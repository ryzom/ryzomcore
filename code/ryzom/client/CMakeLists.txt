# Need clientsheets lib for sheets packer tool
# Need seven_zip for patch_gen and ryzom_installer
ADD_SUBDIRECTORY(src)

IF(WITH_RYZOM_CLIENT)
  #ADD_SUBDIRECTORY(data)

  IF(UNIX AND NOT APPLE)
    ADD_SUBDIRECTORY(unix)
  ENDIF()

  INSTALL(FILES client_default.cfg DESTINATION ${RYZOM_ETC_PREFIX})
ENDIF()
