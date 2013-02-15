FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h)

ADD_EXECUTABLE(zone_dump ${SRC})

TARGET_LINK_LIBRARIES(zone_dump nel3d nelmisc)
NL_DEFAULT_PROPS(zone_dump "NeL, Tools, 3D: zone_dump")
NL_ADD_RUNTIME_FLAGS(zone_dump)

INSTALL(TARGETS zone_dump RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
