FILE(GLOB SRC *.cpp *.h ../s3tc_compressor_lib/*.h ../s3tc_compressor_lib/*.cpp)

ADD_EXECUTABLE(panoply_maker ${SRC})

INCLUDE_DIRECTORIES(${SQUISH_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(panoply_maker ${PLATFORM_LINKFLAGS} ${SQUISH_LIBRARIES} nelmisc nel3d)
NL_DEFAULT_PROPS(panoply_maker "NeL, Tools, 3D: panoply_maker")
NL_ADD_RUNTIME_FLAGS(panoply_maker)

INSTALL(TARGETS panoply_maker RUNTIME DESTINATION bin COMPONENT tools3d)
