FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_ctrl_endpoint ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_ctrl_endpoint PRIVATE NeL::misc)
NL_DEFAULT_PROPS(nl_sample_ctrl_endpoint "NeL, Samples, Misc: Control endpoint")
NL_ADD_RUNTIME_FLAGS(nl_sample_ctrl_endpoint)

INSTALL(TARGETS nl_sample_ctrl_endpoint RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
