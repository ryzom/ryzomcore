FILE(GLOB SRC *.cpp *.h ../ig_lighter_lib/*.cpp ../ig_lighter_lib/*.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(ig_lighter ${SRC})

TARGET_LINK_LIBRARIES(ig_lighter PRIVATE NeL::misc NeL::3d NeL::pacs)
NL_DEFAULT_PROPS(ig_lighter "NeL, Tools, 3D: ig_lighter")
NL_ADD_RUNTIME_FLAGS(ig_lighter)

INSTALL(TARGETS ig_lighter RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
