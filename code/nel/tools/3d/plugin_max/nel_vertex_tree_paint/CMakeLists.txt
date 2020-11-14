FILE(GLOB SRC *.cpp *.h *.def *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(nel_vertex_tree_paint SHARED ${SRC})

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

ADD_DEFINITIONS(${MAXSDK_DEFINITIONS})

INSTALL(TARGETS nel_vertex_tree_paint RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
