FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_font WIN32 ${SRC})

TARGET_COMPILE_DEFINITIONS(nl_sample_font PRIVATE "FONT_DIR=\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_font/\"")

# TODO: hunter TARGET_LINK_LIBRARIES(nl_sample_font PRIVATE nelmisc nel3d)
TARGET_LINK_LIBRARIES(nl_sample_font nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_font "NeL, Samples, 3D: Font")
NL_ADD_RUNTIME_FLAGS(nl_sample_font)

INSTALL(TARGETS nl_sample_font RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
INSTALL(FILES beteckna.ttf DESTINATION ${NL_SHARE_PREFIX}/nl_sample_font COMPONENT samples3d)
