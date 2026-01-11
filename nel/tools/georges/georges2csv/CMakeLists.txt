FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(georges2csv ${SRC})

TARGET_LINK_LIBRARIES(georges2csv nelmisc nelgeorges)
NL_DEFAULT_PROPS(georges2csv "NeL, Tools, Georges: georges2csv")
NL_ADD_RUNTIME_FLAGS(georges2csv)

INSTALL(TARGETS georges2csv RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT toolsgeorges)
