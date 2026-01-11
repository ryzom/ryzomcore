FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(nl_sample_i18n ${SRC})

ADD_DEFINITIONS(-DNL_LANG_DATA="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_i18n/\\"")

TARGET_LINK_LIBRARIES(nl_sample_i18n nelmisc)
NL_DEFAULT_PROPS(nl_sample_i18n "NeL, Samples, Misc: I18N")
NL_ADD_RUNTIME_FLAGS(nl_sample_i18n)

INSTALL(TARGETS nl_sample_i18n RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesmisc)
INSTALL(FILES de.uxt en.uxt fr.uxt DESTINATION ${NL_SHARE_PREFIX}/nl_sample_i18n COMPONENT samplesmisc)
