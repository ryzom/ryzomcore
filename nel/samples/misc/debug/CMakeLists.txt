FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_debug ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_debug nelmisc)
NL_DEFAULT_PROPS(nl_sample_debug "NeL, Samples, Misc: Debugging")
NL_ADD_RUNTIME_FLAGS(nl_sample_debug)

INSTALL(TARGETS nl_sample_debug RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
