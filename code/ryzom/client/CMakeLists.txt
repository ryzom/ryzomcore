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
      IF(MSVC10)
        INSTALL(FILES windows/upgd_nl_msvc10.bat DESTINATION ${RYZOM_GAMES_PREFIX} RENAME upgd_nl.bat)
      ELSEIF(MSVC12)
        INSTALL(FILES windows/upgd_nl_msvc12.bat DESTINATION ${RYZOM_GAMES_PREFIX} RENAME upgd_nl.bat)
      ENDIF()
    ELSE()
      INSTALL(FILES unix/upgd_nl.sh DESTINATION ${RYZOM_GAMES_PREFIX})
    ENDIF()
  ENDIF()
ENDIF()
