FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nl_sample_sound_sources ${SRC})

TARGET_COMPILE_DEFINITIONS(nl_sample_sound_sources PRIVATE "NL_SOUND_DATA=\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_sound/\"")

TARGET_LINK_LIBRARIES(nl_sample_sound_sources nelmisc nelsound)
NL_DEFAULT_PROPS(nl_sample_sound_sources "NeL, Samples: Sound: Sound Sources")
NL_ADD_RUNTIME_FLAGS(nl_sample_sound_sources)

INSTALL(TARGETS nl_sample_sound_sources RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplessound)
INSTALL(DIRECTORY data/
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_sound/data
	COMPONENT samplespacs
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
