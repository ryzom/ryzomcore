FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(zone_check_bind ${SRC})

TARGET_LINK_LIBRARIES(zone_check_bind nel3d nelmisc)
NL_DEFAULT_PROPS(zone_check_bind "NeL, Tools, 3D: zone_check_bind")
NL_ADD_RUNTIME_FLAGS(zone_check_bind)

INSTALL(TARGETS zone_check_bind RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
