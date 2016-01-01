FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(textures_optimizer ${SRC})

TARGET_LINK_LIBRARIES(textures_optimizer nelmisc)
NL_DEFAULT_PROPS(textures_optimizer "NeL, Tools, 3D: Textures optimizer")
NL_ADD_RUNTIME_FLAGS(textures_optimizer)

INSTALL(TARGETS textures_optimizer RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
