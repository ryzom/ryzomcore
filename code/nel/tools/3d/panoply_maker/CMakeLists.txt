FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(panoply_maker ${SRC})

TARGET_LINK_LIBRARIES(panoply_maker s3tc_compressor nelmisc nel3d)
NL_DEFAULT_PROPS(panoply_maker "NeL, Tools, 3D: panoply_maker")
NL_ADD_RUNTIME_FLAGS(panoply_maker)

INSTALL(TARGETS panoply_maker RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
