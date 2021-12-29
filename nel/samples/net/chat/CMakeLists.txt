ADD_EXECUTABLE(nl_sample_chatclient client.cpp kbhit.cpp kbhit.h)

ADD_EXECUTABLE(nl_sample_chatserver WIN32 server.cpp)

ADD_DEFINITIONS(-DCHAT_DIR="\\"${NL_SHARE_ABSOLUTE_PREFIX}/nl_sample_chat/\\"")

TARGET_LINK_LIBRARIES(nl_sample_chatclient nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_chatclient "NeL, Samples, Net, Chat: Chat Client")
NL_ADD_RUNTIME_FLAGS(nl_sample_chatclient)

TARGET_LINK_LIBRARIES(nl_sample_chatserver nelmisc nelnet)
NL_DEFAULT_PROPS(nl_sample_chatserver "NeL, Samples, Net, Chat: Chat Server")
NL_ADD_RUNTIME_FLAGS(nl_sample_chatserver)

INSTALL(TARGETS nl_sample_chatclient nl_sample_chatserver RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT samplesnet)
INSTALL(FILES chat_service.cfg client.cfg DESTINATION ${NL_SHARE_PREFIX}/nl_sample_chat COMPONENT samplesnet)
