FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(tile_utility SHARED ${SRC} tile_utility.rc)

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(tile_utility 
							nelmisc
							nel3d
							nel_mesh_lib
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(tile_utility PROPERTIES SUFFIX ".dlu")
NL_DEFAULT_PROPS(tile_utility "MAX Plugin: Tile Utility")
NL_ADD_RUNTIME_FLAGS(tile_utility)
NL_ADD_LIB_SUFFIX(tile_utility)

INSTALL(TARGETS tile_utility RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
