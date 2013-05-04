FILE(GLOB SRC *.cpp *.h)

ADD_LIBRARY(world_editor_graph_plugin SHARED ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(world_editor_graph_plugin 
							nelmisc
							nel3d
							nelsound
							nelsnd_lowlevel
							ryzom_mission_compiler_lib)
							
NL_DEFAULT_PROPS(world_editor_graph_plugin "Ryzom, Tools, World: World Editor Graph Plugin")
NL_ADD_RUNTIME_FLAGS(world_editor_graph_plugin)
NL_ADD_LIB_SUFFIX(world_editor_graph_plugin)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${MFC_DEFINITIONS} -DWIN32_DLL_EXPORTS)


INSTALL(TARGETS world_editor_graph_plugin LIBRARY DESTINATION ${RYZOM_LIB_PREFIX} RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} ARCHIVE DESTINATION ${RYZOM_LIB_PREFIX} COMPONENT tools3d)

