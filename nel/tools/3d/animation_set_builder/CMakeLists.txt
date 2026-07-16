FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(animation_set_builder ${SRC})

TARGET_LINK_LIBRARIES(animation_set_builder PRIVATE NeL::3d NeL::misc)
NL_DEFAULT_PROPS(animation_set_builder "NeL, Tools, 3D: Animation Set Builder")
NL_ADD_RUNTIME_FLAGS(animation_set_builder)

INSTALL(TARGETS animation_set_builder RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
