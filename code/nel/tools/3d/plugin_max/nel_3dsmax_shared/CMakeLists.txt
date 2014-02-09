FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_3dsmax_shared SHARED ${SRC})

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_3dsmax_shared 
							nelmisc
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							${MAXSDK_LIBRARIES} 
							Version.lib)
							
NL_DEFAULT_PROPS(nel_3dsmax_shared "MAX Plugin: NeL 3DSMAX Shared")
NL_ADD_RUNTIME_FLAGS(nel_3dsmax_shared)
NL_ADD_LIB_SUFFIX(nel_3dsmax_shared)
#SET_TARGET_PROPERTIES(nel_export PROPERTIES SUFFIX ".dlx")

ADD_DEFINITIONS(-DNEL_3DSMAX_SHARED_EXPORTS)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_3dsmax_shared ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS nel_3dsmax_shared RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
