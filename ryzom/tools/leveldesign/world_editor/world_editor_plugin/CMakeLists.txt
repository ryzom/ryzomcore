FILE(GLOB SRC *.cpp *.h)

ADD_LIBRARY(world_editor_plugin SHARED ${SRC} world_editor_plugin.rc)

TARGET_LINK_LIBRARIES(world_editor_plugin 
							nelmisc
							nel3d
							nelsound
							nelsnd_lowlevel
							Version.lib)
							
NL_DEFAULT_PROPS(world_editor_plugin "Ryzom, Tools, World: World Editor Plugin DLL")
NL_ADD_RUNTIME_FLAGS(world_editor_plugin)
NL_ADD_LIB_SUFFIX(world_editor_plugin)

ADD_DEFINITIONS(${MFC_DEFINITIONS} -DOBJECT_VIEWER_LIB_EXPORTS)

IF(WITH_PCH)
  target_precompile_headers(world_editor_plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h)
ENDIF()

INSTALL(TARGETS world_editor_plugin LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT tools3d)
INSTALL(FILES WorldEditorPlugin.cfg DESTINATION ${RYZOM_ETC_PREFIX} COMPONENT tools3d)

