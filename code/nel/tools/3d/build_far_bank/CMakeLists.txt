FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(build_far_bank ${SRC})

TARGET_LINK_LIBRARIES(build_far_bank ${PLATFORM_LINKFLAGS} nel3d nelmisc)
NL_DEFAULT_PROPS(build_far_bank "NeL, Tools, 3D: build_far_bank")
NL_ADD_RUNTIME_FLAGS(build_far_bank)

INSTALL(TARGETS build_far_bank RUNTIME DESTINATION bin COMPONENT tools3d)
