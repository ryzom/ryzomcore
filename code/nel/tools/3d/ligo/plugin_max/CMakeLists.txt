FILE(GLOB SRC *.cpp *.h ../../ig_lighter_lib/*.cpp ../../ig_lighter_lib/*.h)

ADD_LIBRARY(ligoscape_utility SHARED ligoscape_utility.rc ${SRC} ligoscape_utility.def)

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ligoscape_utility 
							${LIBXML2_LIBRARIES} 
							nelmisc 
							nelligo
							nel3d
							nelpacs
							${MAXSDK_LIBRARIES} 
							nel_patch_lib 
							nel_mesh_lib
							nel_3dsmax_shared
							comctl32)
							
SET_TARGET_PROPERTIES(ligoscape_utility PROPERTIES 
  VERSION ${NL_VERSION}
  LINK_FLAGS "/NODEFAULTLIB:libcmt"
  SUFFIX ".dlx"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  PROJECT_LABEL "MAX Plugin: Ligoscape Utility")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ligoscape_utility RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
INSTALL(FILES ligoscript.txt DESTINATION maxplugin/docs)
INSTALL(DIRECTORY scripts/
        DESTINATION maxplugin/scripts/
	COMPONENT libraries
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)
INSTALL(DIRECTORY macroscripts/
        DESTINATION maxplugin/macroscripts/
	COMPONENT libraries
        PATTERN "CVS" EXCLUDE
        PATTERN ".svn" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)