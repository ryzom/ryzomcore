FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(tga_resize ${SRC})

TARGET_LINK_LIBRARIES(tga_resize ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(tga_resize "NeL, Tools, 3D: tga_resize")
NL_ADD_RUNTIME_FLAGS(tga_resize)

INSTALL(TARGETS tga_resize RUNTIME DESTINATION bin COMPONENT tools3d)
