FILE(GLOB SRC *.cpp *.h *.def *.rc)

NL_TARGET_DRIVER(nel_drv_direct3d_win ${SRC})

INCLUDE_DIRECTORIES(BEFORE ${DXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_drv_direct3d_win nel3d nelmisc ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY})

NL_DEFAULT_PROPS(nel_drv_direct3d_win "NeL, Driver, Video: Direct3D")
NL_ADD_RUNTIME_FLAGS(nel_drv_direct3d_win)
NL_ADD_LIB_SUFFIX(nel_drv_direct3d_win)

ADD_DEFINITIONS(-DRIVER_DIRECT3D_EXPORTS)

IF(WITH_PCH)
  target_precompile_headers(nel_drv_direct3d_win PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stddirect3d.h)
ENDIF()

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC_DRIVERS) OR NOT WITH_STATIC_DRIVERS)
  INSTALL(TARGETS nel_drv_direct3d_win LIBRARY DESTINATION ${NL_DRIVER_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_DRIVER_PREFIX} COMPONENT drivers3d)
  IF(WITH_MAXPLUGIN)
    INSTALL(TARGETS nel_drv_direct3d_win RUNTIME DESTINATION maxplugin COMPONENT drivers3d)
  ENDIF()
ENDIF()
