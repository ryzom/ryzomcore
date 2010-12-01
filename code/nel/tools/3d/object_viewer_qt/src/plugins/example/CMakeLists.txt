FILE(GLOB SRC *.cpp *.h)

ADD_LIBRARY(ovqt_plugin_example SHARED ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ovqt_plugin_example nelmisc)
							
NL_DEFAULT_PROPS(ovqt_plugin_example "NeL, Tools, 3D: Object Viewer Qt Plugin: Example")
NL_ADD_RUNTIME_FLAGS(ovqt_plugin_example)
NL_ADD_LIB_SUFFIX(ovqt_plugin_example)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ovqt_plugin_example LIBRARY DESTINATION lib RUNTIME DESTINATION bin ARCHIVE DESTINATION lib COMPONENT tools3d)
