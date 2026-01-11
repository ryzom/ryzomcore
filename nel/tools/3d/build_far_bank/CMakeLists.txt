FILE(GLOB SRC *.cpp *.h *.rc)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(build_far_bank ${SRC})

TARGET_LINK_LIBRARIES(build_far_bank nel3d nelmisc)
NL_DEFAULT_PROPS(build_far_bank "NeL, Tools, 3D: build_far_bank")
NL_ADD_RUNTIME_FLAGS(build_far_bank)

INSTALL(TARGETS build_far_bank RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
