FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_log ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_log nelmisc)
NL_DEFAULT_PROPS(nl_sample_log "NeL, Samples, Misc: Logging")
NL_ADD_RUNTIME_FLAGS(nl_sample_log)

INSTALL(TARGETS nl_sample_log RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
