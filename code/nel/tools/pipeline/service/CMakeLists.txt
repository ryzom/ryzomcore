FILE(GLOB SRC *.cpp *.h)

SOURCE_GROUP("" FILES ${SRC})

FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)
ADD_EXECUTABLE(pipeline_service WIN32 ${SRC})

IF(UNIX)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(pipeline_service pipeline_plugin_library nelmisc nelnet nelgeorges ${Boost_LIBRARIES})
ELSE(UNIX)
  INCLUDE_DIRECTORIES(${BOOST_ROOT})
  TARGET_LINK_LIBRARIES(pipeline_service pipeline_plugin_library nelmisc nelnet nelgeorges)
ENDIF(UNIX)

NL_DEFAULT_PROPS(pipeline_service "Pipeline Service")
NL_ADD_RUNTIME_FLAGS(pipeline_service)

INSTALL(TARGETS pipeline_service RUNTIME DESTINATION bin COMPONENT pipeline)
