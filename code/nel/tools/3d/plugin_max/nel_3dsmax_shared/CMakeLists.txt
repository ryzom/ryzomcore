FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_3dsmax_shared SHARED ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_3dsmax_shared 
							${LIBXML2_LIBRARIES} 
							nelmisc
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							${MAXSDK_LIBRARIES} 
							Version.lib)
							
SET_TARGET_PROPERTIES(nel_3dsmax_shared PROPERTIES 
  VERSION ${NL_VERSION}
  LINK_FLAGS "/NODEFAULTLIB:libcmt"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  PROJECT_LABEL "MAX Plugin: NeL 3DSMAX Shared")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} -DNEL_3DSMAX_SHARED_EXPORTS)

INSTALL(TARGETS nel_3dsmax_shared RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
