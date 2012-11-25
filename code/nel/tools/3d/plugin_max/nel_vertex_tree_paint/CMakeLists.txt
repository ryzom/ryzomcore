FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_vertex_tree_paint SHARED ${SRC} vertex_tree_paint.def vertex_tree_paint.rc)

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_vertex_tree_paint 
							nelmisc
							nel3d
							nel_mesh_lib 
							nel_patch_lib
							nel_3dsmax_shared 
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_vertex_tree_paint PROPERTIES SUFFIX ".dlm")
NL_DEFAULT_PROPS(nel_vertex_tree_paint "MAX Plugin: NeL Vertex Tree Painter")
NL_ADD_RUNTIME_FLAGS(nel_vertex_tree_paint)
NL_ADD_LIB_SUFFIX(nel_vertex_tree_paint)

INSTALL(TARGETS nel_vertex_tree_paint RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
