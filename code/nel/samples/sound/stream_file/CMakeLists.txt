FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nl_sample_stream_file ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_stream_file nelmisc nelsound)
NL_DEFAULT_PROPS(nl_sample_stream_file "NeL, Samples: Sound: Stream File")
NL_ADD_RUNTIME_FLAGS(nl_sample_stream_file)

INSTALL(TARGETS nl_sample_stream_file RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplessound)

