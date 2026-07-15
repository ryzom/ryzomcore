FILE(GLOB SRC *.cpp *.h *.def *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(nel_3dsmax_shared SHARED ${SRC})

TARGET_INCLUDE_DIRECTORIES(nel_3dsmax_shared PRIVATE ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_3dsmax_shared 
							NeL::misc
							NeL::3d
							nel_mesh_lib 
							nel_patch_lib 
							${MAXSDK_LIBRARIES} 
							Version.lib)
							
NL_DEFAULT_PROPS(nel_3dsmax_shared "MAX Plugin: NeL 3ds Max Shared")
NL_ADD_RUNTIME_FLAGS(nel_3dsmax_shared)
NL_ADD_LIB_SUFFIX(nel_3dsmax_shared)
#SET_TARGET_PROPERTIES(nel_export PROPERTIES SUFFIX ".dlx")

TARGET_COMPILE_DEFINITIONS(nel_3dsmax_shared PRIVATE NEL_3DSMAX_SHARED_EXPORTS ${MAXSDK_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(nel_3dsmax_shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS nel_3dsmax_shared RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
