FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nl_sample_sound_sources ${SRC})

ADD_DEFINITIONS(-DNL_SOUND_DATA="\\"${NL_SHARE_PREFIX}/nl_sample_sound/\\"")

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nl_sample_sound_sources ${LIBXML2_LIBRARIES} ${PLATFORM_LINKFLAGS} nelmisc nelsound nelsnd_lowlevel)

NL_DEFAULT_PROPS(nl_sample_sound_sources "NeL, Samples: Sound System")
NL_ADD_RUNTIME_FLAGS(nl_sample_sound_sources)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nl_sample_sound_sources RUNTIME DESTINATION bin COMPONENT samplessound)
INSTALL(DIRECTORY data/ 
	DESTINATION share/nel/nl_sample_sound/data
	COMPONENT samplespacs
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
