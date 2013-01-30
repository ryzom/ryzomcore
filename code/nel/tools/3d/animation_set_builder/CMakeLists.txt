FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(animation_set_builder ${SRC})

TARGET_LINK_LIBRARIES(animation_set_builder nel3d nelmisc)
NL_DEFAULT_PROPS(animation_set_builder "NeL, Tools, 3D: anim_set_builder")
NL_ADD_RUNTIME_FLAGS(animation_set_builder)

INSTALL(TARGETS animation_set_builder RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
