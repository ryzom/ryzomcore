FILE(GLOB SRC *.cpp *.h *.def)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/nel_paint.cpp)

ADD_LIBRARY(nel_patch_paint SHARED ${SRC} nel_patch_paint.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_paint 
							${LIBXML2_LIBRARIES} 
							nelmisc
							nelligo
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared 
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_paint PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt"
	VERSION ${NL_VERSION}
	SUFFIX ".dlm"
	DEBUG_POSTFIX "_d"
	RELEASE_POSTFIX "_r"
	PROJECT_LABEL "MAX Plugin: NeL Patch Painter")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nel_patch_paint RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
INSTALL(FILES keys.cfg DESTINATION maxplugin/plugins )