FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_font_perf ${SRC})

ADD_DEFINITIONS(-DFONT_DIR="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_font_perf/\\"")

TARGET_LINK_LIBRARIES(nl_sample_font_perf nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_font_perf "NeL, Samples, 3D: Font Performance Test")
NL_ADD_RUNTIME_FLAGS(nl_sample_font_perf)

INSTALL(TARGETS nl_sample_font_perf RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
INSTALL(FILES beteckna.ttf DESTINATION ${NL_SHARE_PREFIX}/nl_sample_font_perf COMPONENT samples3d)


