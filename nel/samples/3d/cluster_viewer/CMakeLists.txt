FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_clusterview WIN32 ${SRC})

ADD_DEFINITIONS(-DCV_DIR="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_clusterview/\\"")

TARGET_LINK_LIBRARIES(nl_sample_clusterview nelmisc nel3d)
NL_ADD_RUNTIME_FLAGS(nl_sample_clusterview)
NL_DEFAULT_PROPS(nl_sample_clusterview "NeL, Samples, 3D: Cluster Viewer")

INSTALL(TARGETS nl_sample_clusterview RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
INSTALL(FILES main.cvs readme.txt DESTINATION ${NL_SHARE_PREFIX}/nl_sample_clusterview COMPONENT samples3d)
INSTALL(DIRECTORY groups/
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_clusterview/groups
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY fonts/
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_clusterview/fonts
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY max/
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_clusterview/max
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY shapes/
	DESTINATION ${NL_SHARE_PREFIX}/nl_sample_clusterview/shapes
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
