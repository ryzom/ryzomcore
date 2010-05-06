FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(tile_utility SHARED ${SRC} tile_utility.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(tile_utility 
							${LIBXML2_LIBRARIES} 
							nelmisc
							nel3d
							nel_mesh_lib
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(tile_utility PROPERTIES 
	VERSION ${NL_VERSION}
	LINK_FLAGS "/NODEFAULTLIB:libcmt"
	SUFFIX ".dlu"
	DEBUG_POSTFIX "_d"
	RELEASE_POSTFIX "_r"
	PROJECT_LABEL "MAX Plugin: Tile Utility")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS tile_utility RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
