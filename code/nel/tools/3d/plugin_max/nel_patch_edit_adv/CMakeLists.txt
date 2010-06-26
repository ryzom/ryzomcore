FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_patch_edit_adv SHARED ${SRC} mods.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_edit_adv 
							${LIBXML2_LIBRARIES} 
							nelmisc
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_edit_adv PROPERTIES 
  VERSION ${NL_VERSION}
  LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
  LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}"
  SUFFIX ".dlm"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  PROJECT_LABEL "MAX Plugin: NeL Patch Edit (Adv)")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nel_patch_edit_adv RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
