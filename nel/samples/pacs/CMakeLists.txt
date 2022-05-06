FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nl_sample_pacs WIN32 ${SRC})

ADD_DEFINITIONS(-DNL_PACS_DATA="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_pacs/\\"")

TARGET_LINK_LIBRARIES(nl_sample_pacs nelmisc nelpacs nel3d)
NL_DEFAULT_PROPS(nl_sample_pacs "NeL, Samples: PACS")
NL_ADD_RUNTIME_FLAGS(nl_sample_pacs)

INSTALL(TARGETS nl_sample_pacs RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplespacs)
INSTALL(FILES readme.txt DESTINATION ${NL_SHARE_PREFIX}/nl_sample_pacs COMPONENT samplespacs)
INSTALL(DIRECTORY shapes/
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_pacs/shapes
	COMPONENT samplespacs
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
