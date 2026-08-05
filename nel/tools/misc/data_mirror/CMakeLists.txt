FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(data_mirror WIN32 ${SRC})
SET_TARGET_PROPERTIES(data_mirror PROPERTIES NL_MFC_UNICODE_APP ON) # unicode MFC GUI: pre-VS2010 links need the explicit wWinMain entry (see NL_DEFAULT_PROPS)

TARGET_LINK_LIBRARIES(data_mirror PRIVATE NeL::misc)
NL_DEFAULT_PROPS(data_mirror "NeL, Tools, Misc: data_mirror")
NL_ADD_RUNTIME_FLAGS(data_mirror)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

INSTALL(TARGETS data_mirror RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
