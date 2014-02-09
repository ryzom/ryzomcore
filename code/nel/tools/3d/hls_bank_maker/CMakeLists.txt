FILE(GLOB SRC *.cpp *.h ../panoply_maker/hls_bank_texture_info.cpp ../panoply_maker/hls_bank_texture_info.h)
		
ADD_EXECUTABLE(hls_bank_maker ${SRC})

TARGET_LINK_LIBRARIES(hls_bank_maker s3tc_compressor nelmisc nel3d)
NL_DEFAULT_PROPS(hls_bank_maker "NeL, Tools, 3D: hls_bank_maker")
NL_ADD_RUNTIME_FLAGS(hls_bank_maker)

INSTALL(TARGETS hls_bank_maker RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
