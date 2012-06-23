FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(pipeline_plugin_nel MODULE ${SRC})

TARGET_LINK_LIBRARIES(pipeline_plugin_nel pipeline_plugin_library nelmisc)

NL_DEFAULT_PROPS(pipeline_plugin_nel "Pipeline Plugin NeL")
NL_ADD_LIB_SUFFIX(pipeline_plugin_nel)
NL_ADD_RUNTIME_FLAGS(pipeline_plugin_nel)

INSTALL(TARGETS pipeline_plugin_nel LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin COMPONENT pipeline)
