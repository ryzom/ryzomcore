FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nl_sample_stream_ogg_vorbis ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_stream_ogg_vorbis nelmisc nelsound)
NL_DEFAULT_PROPS(nl_sample_stream_ogg_vorbis "NeL, Samples: Sound: Stream OGG Vorbis")
NL_ADD_RUNTIME_FLAGS(nl_sample_stream_ogg_vorbis)

INSTALL(TARGETS nl_sample_stream_ogg_vorbis RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplessound)

