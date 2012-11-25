FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_strings ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_strings nelmisc)
NL_DEFAULT_PROPS(nl_sample_strings "NeL, Samples, Misc: Strings")
NL_ADD_RUNTIME_FLAGS(nl_sample_strings)

INSTALL(TARGETS nl_sample_strings RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
