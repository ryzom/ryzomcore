FILE(GLOB SRC *.cpp *.h ../zone_lib/*.cpp ../zone_lib/*.h)

ADD_EXECUTABLE(zone_ig_lighter ${SRC})

TARGET_LINK_LIBRARIES(zone_ig_lighter nelmisc nel3d)
NL_DEFAULT_PROPS(zone_ig_lighter "NeL, Tools, 3D: zone_ig_lighter")
NL_ADD_RUNTIME_FLAGS(zone_ig_lighter)

INSTALL(TARGETS zone_ig_lighter RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
