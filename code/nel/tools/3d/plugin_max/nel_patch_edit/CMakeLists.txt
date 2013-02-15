FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_patch_edit SHARED ${SRC} mods.rc)

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_edit 
							nelmisc 
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_edit PROPERTIES SUFFIX ".dlm")
NL_DEFAULT_PROPS(nel_patch_edit "MAX Plugin: NeL Patch Editor")
NL_ADD_RUNTIME_FLAGS(nel_patch_edit)
NL_ADD_LIB_SUFFIX(nel_patch_edit)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_patch_edit ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS nel_patch_edit RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
