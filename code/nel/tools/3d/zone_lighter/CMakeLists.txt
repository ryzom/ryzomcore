FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h)

ADD_EXECUTABLE(zone_lighter ${SRC})

TARGET_LINK_LIBRARIES(zone_lighter nel3d nelgeorges nelmisc)
NL_DEFAULT_PROPS(zone_lighter "NeL, Tools, 3D: zone_lighter")
NL_ADD_RUNTIME_FLAGS(zone_lighter)

INSTALL(TARGETS zone_lighter RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
