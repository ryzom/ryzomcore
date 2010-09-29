FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_configfile ${SRC})

ADD_DEFINITIONS(-DNL_SAMPLE_CFG="\\"${NL_SHARE_PREFIX}/nl_sample_configfile/\\"")

TARGET_LINK_LIBRARIES(nl_sample_configfile nelmisc)
NL_DEFAULT_PROPS(nl_sample_configfile "NeL, Samples, Misc: Config Files")
NL_ADD_RUNTIME_FLAGS(nl_sample_configfile)

INSTALL(TARGETS nl_sample_configfile RUNTIME DESTINATION bin COMPONENT samplesmisc)
INSTALL(FILES simpletest.txt DESTINATION share/nel/nl_sample_configfile COMPONENT samplesmisc)
