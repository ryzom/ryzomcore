FILE(GLOB SRC *.cpp *.h *.rc *.rc2)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(data_mirror WIN32 ${SRC})

TARGET_LINK_LIBRARIES(data_mirror nelmisc)
NL_DEFAULT_PROPS(data_mirror "NeL, Tools, Misc: data_mirror")
NL_ADD_RUNTIME_FLAGS(data_mirror)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

INSTALL(TARGETS data_mirror RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsmisc)
