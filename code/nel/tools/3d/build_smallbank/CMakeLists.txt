FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(build_smallbank ${SRC})

TARGET_LINK_LIBRARIES(build_smallbank nel3d nelmisc)
NL_DEFAULT_PROPS(build_smallbank "NeL, Tools, 3D: build_smallbank")
NL_ADD_RUNTIME_FLAGS(build_smallbank)

INSTALL(TARGETS build_smallbank RUNTIME DESTINATION bin COMPONENT tools3d)
