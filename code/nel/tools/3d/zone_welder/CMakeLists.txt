FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h)

ADD_EXECUTABLE(zone_welder ${SRC})

TARGET_LINK_LIBRARIES(zone_welder nel3d nelmisc)
NL_DEFAULT_PROPS(zone_welder "NeL, Tools, 3D: zone_welder")
NL_ADD_RUNTIME_FLAGS(zone_welder)

INSTALL(TARGETS zone_welder RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
