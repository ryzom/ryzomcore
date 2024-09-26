FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_types_check ${SRC})

TARGET_LINK_LIBRARIES(nl_sample_types_check nelmisc)
NL_DEFAULT_PROPS(nl_sample_types_check "Samples, MISC: Types check sample")
NL_ADD_RUNTIME_FLAGS(nl_sample_types_check)

INSTALL(TARGETS nl_sample_types_check RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
