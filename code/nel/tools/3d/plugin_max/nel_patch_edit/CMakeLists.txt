FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_patch_edit SHARED ${SRC} mods.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_edit 
							${LIBXML2_LIBRARIES} 
							nelmisc 
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_edit PROPERTIES 
  VERSION ${NL_VERSION}
  LINK_FLAGS "/NODEFAULTLIB:libcmt"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  SUFFIX ".dlm"
  PROJECT_LABEL "MAX Plugin: NeL Patch Editor")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nel_patch_edit RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
