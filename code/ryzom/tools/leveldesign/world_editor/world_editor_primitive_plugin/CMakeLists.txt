FILE(GLOB SRC *.cpp *.h)

ADD_LIBRARY(world_editor_primitive_plugin SHARED ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(world_editor_primitive_plugin 
							nelmisc
							nel3d
							nelsound
							nelsnd_lowlevel)
							
NL_DEFAULT_PROPS(world_editor_primitive_plugin "Ryzom, Tools, World: World Editor Primitive Plugin")
NL_ADD_RUNTIME_FLAGS(world_editor_primitive_plugin)
NL_ADD_LIB_SUFFIX(world_editor_primitive_plugin)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${MFC_DEFINITIONS} -DWIN32_DLL_EXPORTS)


INSTALL(TARGETS world_editor_primitive_plugin LIBRARY DESTINATION lib RUNTIME DESTINATION bin ARCHIVE DESTINATION lib COMPONENT tools3d)

