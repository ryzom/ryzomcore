FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_font WIN32 ${SRC})

ADD_DEFINITIONS(-DFONT_DIR="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_font/\\"")

TARGET_LINK_LIBRARIES(nl_sample_font nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_font "NeL, Samples, 3D: Font")
NL_ADD_RUNTIME_FLAGS(nl_sample_font)

INSTALL(TARGETS nl_sample_font RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
INSTALL(FILES beteckna.ttf DESTINATION ${NL_SHARE_PREFIX}/nl_sample_font COMPONENT samples3d)
