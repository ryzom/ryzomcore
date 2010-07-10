FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_patch_edit SHARED ${SRC} mods.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_edit 
							${LIBXML2_LIBRARIES} 
							nelmisc 
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_edit PROPERTIES 
  VERSION ${NL_VERSION}
  LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
  LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}"
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  SUFFIX ".dlm"
  PROJECT_LABEL "MAX Plugin: NeL Patch Editor")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(nel_patch_edit ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS nel_patch_edit RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
