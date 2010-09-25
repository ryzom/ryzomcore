FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(log_analyser WIN32 ${SRC} log_analyser.rc)

TARGET_LINK_LIBRARIES(log_analyser ${PLATFORM_LINKFLAGS} nelmisc)
NL_DEFAULT_PROPS(log_analyser "NeL, Tools, Misc: log_analyser")
NL_ADD_RUNTIME_FLAGS(log_analyser)

ADD_DEFINITIONS(${MFC_DEFINITIONS})

INSTALL(TARGETS log_analyser RUNTIME DESTINATION bin COMPONENT toolsmisc)
