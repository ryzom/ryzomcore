FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(zone_dependencies ${SRC})

TARGET_LINK_LIBRARIES(zone_dependencies nel3d nelgeorges nelmisc)
NL_DEFAULT_PROPS(zone_dependencies "NeL, Tools, 3D: zone_dependencies")
NL_ADD_RUNTIME_FLAGS(zone_dependencies)

INSTALL(TARGETS zone_dependencies RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
