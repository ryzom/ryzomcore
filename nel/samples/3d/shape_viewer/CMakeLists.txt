FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_shapeview WIN32 ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_shapeview nelmisc nel3d)
NL_DEFAULT_PROPS(nl_sample_shapeview "NeL, Samples, 3D: Shape Viewer")
NL_ADD_RUNTIME_FLAGS(nl_sample_shapeview)

INSTALL(TARGETS nl_sample_shapeview RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samples3d)
