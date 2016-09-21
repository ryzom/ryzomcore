# Need clientsheets lib for sheets packer tool
# Need seven_zip for patch_gen and ryzom_installer
ADD_SUBDIRECTORY(src)

IF(WITH_RYZOM_CLIENT)
  #ADD_SUBDIRECTORY(data)

  IF(UNIX AND NOT APPLE)
    ADD_SUBDIRECTORY(unix)
  ENDIF()

  INSTALL(FILES client_default.cfg DESTINATION ${RYZOM_ETC_PREFIX})

  IF(WITH_RYZOM_PATCH)
    IF(APPLE)
      INSTALL(FILES macosx/upgd_nl.sh DESTINATION ${RYZOM_GAMES_PREFIX})
    ELSEIF(WIN32)
      INSTALL(FILES windows/upgd_nl.bat DESTINATION ${RYZOM_GAMES_PREFIX})
    ELSE()
      INSTALL(FILES unix/upgd_nl.sh DESTINATION ${RYZOM_GAMES_PREFIX})
    ENDIF()
ENDIF()
