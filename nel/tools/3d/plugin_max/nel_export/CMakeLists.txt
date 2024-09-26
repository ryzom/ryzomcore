FILE(GLOB SRC *.cpp *.h *.def ../../ig_lighter_lib/*.cpp ../../ig_lighter_lib/*.cpp)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/last_lightmap.cpp
						${CMAKE_CURRENT_SOURCE_DIR}/nel_export_lightmap_v1.cpp
						${CMAKE_CURRENT_SOURCE_DIR}/nel_export_lightmap_v2.cpp)

ADD_LIBRARY(nel_export SHARED ${SRC} nel_export.rc)

INCLUDE_DIRECTORIES(${MAXSDK_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_export
							nelmisc 
							nelpacs
							nel3d
							nel_mesh_lib 
							nel_patch_lib
							nel_3dsmax_shared 
							object_viewer_dll 
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)

NL_DEFAULT_PROPS(nel_export "MAX Plugin: NeL Export")
NL_ADD_RUNTIME_FLAGS(nel_export)
NL_ADD_LIB_SUFFIX(nel_export)
SET_TARGET_PROPERTIES(nel_export PROPERTIES SUFFIX ".dlu")

ADD_DEFINITIONS(${MAXSDK_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_export ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.h ${CMAKE_CURRENT_SOURCE_DIR}/std_afx.cpp)
ENDIF()

INSTALL(TARGETS nel_export RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION ${NL_LIB_PREFIX} ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT libraries)
