FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(build_clod_bank ${SRC})

TARGET_LINK_LIBRARIES(build_clod_bank nel3d nelmisc)
NL_DEFAULT_PROPS(build_clod_bank "NeL, Tools, 3D: build_clod_bank")
NL_ADD_RUNTIME_FLAGS(build_clod_bank)

INSTALL(TARGETS build_clod_bank RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
