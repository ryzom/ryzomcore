FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(log_analyser WIN32 ${SRC})
SET_TARGET_PROPERTIES(log_analyser PROPERTIES NL_MFC_UNICODE_APP ON) # unicode MFC GUI: pre-VS2010 links need the explicit wWinMain entry (see NL_DEFAULT_PROPS)

TARGET_LINK_LIBRARIES(log_analyser PRIVATE NeL::misc)
NL_DEFAULT_PROPS(log_analyser "NeL, Tools, Misc: log_analyser")
NL_ADD_RUNTIME_FLAGS(log_analyser)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

INSTALL(TARGETS log_analyser RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
