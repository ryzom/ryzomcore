FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(build_shadow_skin ${SRC})

TARGET_LINK_LIBRARIES(build_shadow_skin nelmisc nel3d)
NL_DEFAULT_PROPS(build_shadow_skin "NeL, Tools, 3D: build_shadow_skin")
NL_ADD_RUNTIME_FLAGS(build_shadow_skin)

INSTALL(TARGETS build_shadow_skin RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
