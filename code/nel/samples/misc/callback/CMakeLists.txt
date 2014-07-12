FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_callback ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_callback nelmisc)
NL_DEFAULT_PROPS(nl_sample_callback "NeL, Samples, Misc: Callback")
NL_ADD_RUNTIME_FLAGS(nl_sample_callback)

INSTALL(TARGETS nl_sample_callback RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
