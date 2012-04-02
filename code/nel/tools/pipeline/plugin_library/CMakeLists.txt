FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

NL_TARGET_LIB(pipeline_plugin_library ${SRC})

TARGET_LINK_LIBRARIES(pipeline_plugin_library nelmisc)
SET_TARGET_PROPERTIES(pipeline_plugin_library PROPERTIES LINK_INTERFACE_LIBRARIES "")
NL_DEFAULT_PROPS(pipeline_plugin_library "Pipeline Plugin Library")
NL_ADD_LIB_SUFFIX(pipeline_plugin_library)
NL_ADD_RUNTIME_FLAGS(pipeline_plugin_library)

IF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
  INSTALL(TARGETS pipeline_plugin_library LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
ENDIF((WITH_INSTALL_LIBRARIES AND WITH_STATIC) OR NOT WITH_STATIC)
