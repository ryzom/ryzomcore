FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(build_interface ${SRC})

TARGET_LINK_LIBRARIES(build_interface ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(build_interface "NeL, Tools, 3D: build_interface")
NL_ADD_RUNTIME_FLAGS(build_interface)

INSTALL(TARGETS build_interface RUNTIME DESTINATION bin COMPONENT tools3d)
