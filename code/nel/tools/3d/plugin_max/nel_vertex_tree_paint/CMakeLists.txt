FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_vertex_tree_paint SHARED ${SRC} vertex_tree_paint.def vertex_tree_paint.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_vertex_tree_paint 
							${LIBXML2_LIBRARIES} 
							nelmisc
							nel3d
							nel_mesh_lib 
							nel_patch_lib
							nel_3dsmax_shared 
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_vertex_tree_paint PROPERTIES 
  VERSION ${NL_VERSION}
  LINK_FLAGS "/NODEFAULTLIB:libcmt"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  SUFFIX ".dlm"
  PROJECT_LABEL "MAX Plugin: NeL Vertex Tree Painter")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nel_vertex_tree_paint RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
