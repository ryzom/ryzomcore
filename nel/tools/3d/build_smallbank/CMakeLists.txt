FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(build_smallbank ${SRC})

TARGET_LINK_LIBRARIES(build_smallbank nel3d nelmisc)
NL_DEFAULT_PROPS(build_smallbank "NeL, Tools, 3D: build_smallbank")
NL_ADD_RUNTIME_FLAGS(build_smallbank)

INSTALL(TARGETS build_smallbank RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
