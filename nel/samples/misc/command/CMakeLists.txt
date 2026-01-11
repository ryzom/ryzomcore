FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_command ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_command nelmisc)
NL_DEFAULT_PROPS(nl_sample_command "NeL, Samples, Misc: Commands")
NL_ADD_RUNTIME_FLAGS(nl_sample_command)

INSTALL(TARGETS nl_sample_command RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
