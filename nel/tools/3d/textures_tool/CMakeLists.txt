FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(textures_tool ${SRC})

TARGET_LINK_LIBRARIES(textures_tool nelmisc)
NL_DEFAULT_PROPS(textures_tool "NeL, Tools, 3D: Textures Tool")
NL_ADD_RUNTIME_FLAGS(textures_tool)

INSTALL(TARGETS textures_tool RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
