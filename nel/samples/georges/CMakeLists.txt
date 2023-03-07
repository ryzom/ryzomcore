FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_georges ${SRC})

TARGET_COMPILE_DEFINITIONS(nl_sample_georges PRIVATE "GF_DIR=\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_georges/\"")

TARGET_LINK_LIBRARIES(nl_sample_georges nelgeorges nelmisc)
NL_DEFAULT_PROPS(nl_sample_georges "NeL, Samples: Georges")
NL_ADD_RUNTIME_FLAGS(nl_sample_georges)

INSTALL(TARGETS nl_sample_georges RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesgeorges)
INSTALL(FILES boolean.typ coolfilesinfo.dfn default.sample_config int.typ positiondata.dfn sample_config.dfn string.typ sheet_id.bin
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_georges/ COMPONENT samplesgeorges)
