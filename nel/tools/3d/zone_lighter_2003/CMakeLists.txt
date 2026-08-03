FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(zone_lighter_2003 ${SRC})

TARGET_LINK_LIBRARIES(zone_lighter_2003 PRIVATE NeL::3d NeL::georges NeL::misc)
NL_DEFAULT_PROPS(zone_lighter_2003 "NeL, Tools, 3D: zone_lighter_2003 (2003-era relighter)")
NL_ADD_RUNTIME_FLAGS(zone_lighter_2003)

INSTALL(TARGETS zone_lighter_2003 RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
