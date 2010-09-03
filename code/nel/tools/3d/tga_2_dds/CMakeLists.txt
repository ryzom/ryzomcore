FILE(GLOB SRC *.cpp *.h ../s3tc_compressor_lib/*.cpp ../s3tc_compressor_lib/*.h)

ADD_EXECUTABLE(tga2dds ${SRC})

INCLUDE_DIRECTORIES(${SQUISH_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(tga2dds ${PLATFORM_LINKFLAGS} ${SQUISH_LIBRARY} nelmisc nel3d)
NL_DEFAULT_PROPS(tga2dds "NeL, Tools, 3D: tga2dds")
NL_ADD_RUNTIME_FLAGS(tga2dds)

INSTALL(TARGETS tga2dds RUNTIME DESTINATION bin COMPONENT tools3d)
