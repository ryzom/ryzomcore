FILE(GLOB SRC *.cpp *.h *.def ../../ig_lighter_lib/*.cpp ../../ig_lighter_lib/*.cpp)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/last_lightmap.cpp
						${CMAKE_CURRENT_SOURCE_DIR}/nel_export_lightmap_v1.cpp
						${CMAKE_CURRENT_SOURCE_DIR}/nel_export_lightmap_v2.cpp)

ADD_LIBRARY(nel_export SHARED ${SRC} nel_export.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_export
							${LIBXML2_LIBRARIES} 
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
							
SET_TARGET_PROPERTIES(nel_export PROPERTIES
  VERSION ${NL_VERSION} 
  LINK_FLAGS "/NODEFAULTLIB:libcmt"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  SUFFIX ".dlu"
  PROJECT_LABEL "MAX Plugin: NeL Export")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nel_export RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
