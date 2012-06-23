FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(pipeline_plugin_max MODULE ${SRC})

TARGET_LINK_LIBRARIES(pipeline_plugin_max pipeline_plugin_library nelmisc)

NL_DEFAULT_PROPS(pipeline_plugin_max "Pipeline Plugin Max")
NL_ADD_LIB_SUFFIX(pipeline_plugin_max)
NL_ADD_RUNTIME_FLAGS(pipeline_plugin_max)

INSTALL(TARGETS pipeline_plugin_max LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin COMPONENT pipeline)
