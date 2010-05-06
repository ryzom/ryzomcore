FILE(GLOB SRC *.cpp *.h *.def)

ADD_LIBRARY(nel_patch_converter SHARED ${SRC} nel_patch_converter.rc)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MAXSDK_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nel_patch_converter
							${LIBXML2_LIBRARIES} 
							nelmisc 
							nel3d
							nel_mesh_lib 
							nel_patch_lib 
							nel_3dsmax_shared
							${MAXSDK_LIBRARIES} 
							Version.lib
							comctl32.lib)
							
SET_TARGET_PROPERTIES(nel_patch_converter PROPERTIES 
  VERSION ${NL_VERSION}
  DEBUG_POSTFIX "_d"
  RELEASE_POSTFIX "_r"
  SUFFIX ".dlm"
  PROJECT_LABEL "MAX Plugin: NeL Patch Converter")

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nel_patch_converter RUNTIME DESTINATION maxplugin/plugins LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
