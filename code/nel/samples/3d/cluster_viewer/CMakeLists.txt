FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_clusterview ${SRC})

ADD_DEFINITIONS(-DCV_DIR="\\"${NL_SHARE_PREFIX}/nl_sample_clusterview/\\"")

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nl_sample_clusterview ${LIBXML2_LIBRARIES} ${PLATFORM_LINKFLAGS} nelmisc nel3d)
IF(WIN32)
  SET_TARGET_PROPERTIES(nl_sample_clusterview PROPERTIES 
    LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
    PROJECT_LABEL "Samples, 3D: Cluster Viewer")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nl_sample_clusterview RUNTIME DESTINATION bin COMPONENT samples3d)
INSTALL(FILES main.cvs readme.txt DESTINATION share/nel/nl_sample_clusterview COMPONENT samples3d)
INSTALL(DIRECTORY groups/ 
	DESTINATION share/nel/nl_sample_clusterview/groups
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY fonts/ 
	DESTINATION share/nel/nl_sample_clusterview/fonts
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY max/ 
	DESTINATION share/nel/nl_sample_clusterview/max
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY shapes/ 
	DESTINATION share/nel/nl_sample_clusterview/shapes
	COMPONENT samples3d
	PATTERN "CVS" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)
