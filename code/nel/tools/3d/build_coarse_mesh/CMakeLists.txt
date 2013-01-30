FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(build_coarse_mesh ${SRC})

TARGET_LINK_LIBRARIES(build_coarse_mesh nel3d nelmisc)
NL_DEFAULT_PROPS(build_coarse_mesh "NeL, Tools, 3D: build_coarse_mesh")
NL_ADD_RUNTIME_FLAGS(build_coarse_mesh)

INSTALL(TARGETS build_coarse_mesh RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
