FILE(GLOB SRC *.cpp *.h *.def *.rc *.rc2)

LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/nel_paint.cpp)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(nel_patch_paint SHARED ${SRC})

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_patch_paint 
							nelmisc
							nelligo
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared 
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_paint PROPERTIES SUFFIX ".dlm")
NL_DEFAULT_PROPS(nel_patch_paint "MAX Plugin: NeL Patch Painter")
NL_ADD_RUNTIME_FLAGS(nel_patch_paint)
NL_ADD_LIB_SUFFIX(nel_patch_paint)

ADD_DEFINITIONS(${MAXSDK_DEFINITIONS})

INSTALL(TARGETS nel_patch_paint RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
INSTALL(FILES keys.cfg DESTINATION maxplugin/plugins )