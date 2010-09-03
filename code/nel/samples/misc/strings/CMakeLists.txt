FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_strings ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_strings ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(nl_sample_strings "NeL, Samples, Misc: Strings")
NL_ADD_RUNTIME_FLAGS(nl_sample_strings)

INSTALL(TARGETS nl_sample_strings RUNTIME DESTINATION bin COMPONENT samplesmisc)
