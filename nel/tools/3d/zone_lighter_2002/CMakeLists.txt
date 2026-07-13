FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(zone_lighter_2002 ${SRC})

TARGET_LINK_LIBRARIES(zone_lighter_2002 PRIVATE NeL::3d NeL::georges NeL::misc)
NL_DEFAULT_PROPS(zone_lighter_2002 "NeL, Tools, 3D: zone_lighter_2002 (2002-era raytrace+oversampling relighter)")
NL_ADD_RUNTIME_FLAGS(zone_lighter_2002)

INSTALL(TARGETS zone_lighter_2002 RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)

