
IF(WITH_RYZOM_CLIENT)
  ADD_SUBDIRECTORY( client_patcher )
  
  IF( WITH_QT )
    ADD_SUBDIRECTORY( client_config_qt )
  ENDIF( WITH_QT )
ENDIF(WITH_RYZOM_CLIENT)

ADD_SUBDIRECTORY( r2_islands_textures )
