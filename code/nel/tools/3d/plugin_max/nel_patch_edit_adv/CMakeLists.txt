FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_patch_edit_adv SHARED ${SRC} mods.rc)

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_edit_adv 
							nelmisc
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_edit_adv PROPERTIES SUFFIX ".dlm")
NL_DEFAULT_PROPS(nel_patch_edit_adv "MAX Plugin: NeL Patch Edit (Adv)")
NL_ADD_RUNTIME_FLAGS(nel_patch_edit_adv)
NL_ADD_LIB_SUFFIX(nel_patch_edit_adv)

INSTALL(TARGETS nel_patch_edit_adv RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
