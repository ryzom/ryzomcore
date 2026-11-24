FILE(GLOB SRC *.cpp *.manifest)

ADD_EXECUTABLE(driver_check ${SRC})

TARGET_LINK_LIBRARIES(driver_check PRIVATE NeL::misc NeL::3d)
NL_DEFAULT_PROPS(driver_check "NeL, Tools, 3D: driver_check")
NL_ADD_RUNTIME_FLAGS(driver_check)

# the driver is a module that the tool loads when it runs, or a library to link
IF(WITH_STATIC_DRIVERS)
  NL_ADD_STATIC_VID_DRIVERS(driver_check)
ELSEIF(WIN32)
  ADD_DEPENDENCIES(driver_check nel_drv_opengles3_win)
ELSE()
  ADD_DEPENDENCIES(driver_check nel_drv_opengles3)
ENDIF()

INSTALL(TARGETS driver_check RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
