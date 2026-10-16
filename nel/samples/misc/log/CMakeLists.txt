FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_log ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_log PRIVATE NeL::misc)
NL_DEFAULT_PROPS(nl_sample_log "NeL, Samples, Misc: Logging")
NL_ADD_RUNTIME_FLAGS(nl_sample_log)

INSTALL(TARGETS nl_sample_log RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
