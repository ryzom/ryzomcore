FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_LIBRARY(pipeline_plugin_library STATIC ${SRC})

TARGET_LINK_LIBRARIES(pipeline_plugin_library nelmisc)

NL_DEFAULT_PROPS(pipeline_plugin_library "Pipeline Plugin Library")
NL_ADD_LIB_SUFFIX(pipeline_plugin_library)
NL_ADD_RUNTIME_FLAGS(pipeline_plugin_library)

INSTALL(TARGETS pipeline_plugin_library RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib COMPONENT libraries)
