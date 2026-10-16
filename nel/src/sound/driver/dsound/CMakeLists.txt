FILE(GLOB SRC *.cpp *.h *.def *.rc)

NL_TARGET_DRIVER(nel_drv_dsound_win ${SRC})

INCLUDE_DIRECTORIES(BEFORE ${DXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_drv_dsound_win nelmisc nelsnd_lowlevel ${DXSDK_DSOUND_LIBRARY})

NL_DEFAULT_PROPS(nel_drv_dsound_win "NeL, Driver, Sound: DirectSound")
NL_ADD_RUNTIME_FLAGS(nel_drv_dsound_win)
NL_ADD_LIB_SUFFIX(nel_drv_dsound_win)

IF(WITH_PCH)
  target_precompile_headers(nel_drv_dsound_win PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stddsound.h)
ENDIF()

IF(WITH_INSTALL_LIBRARIES OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS nel_drv_dsound_win RUNTIME DESTINATION ${NL_DRIVER_PREFIX} LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT driverssound)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS nel_drv_dsound_win RUNTIME DESTINATION maxplugin COMPONENT driverssound)
  ENDIF()
ENDIF()
