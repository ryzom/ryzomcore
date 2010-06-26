

ADD_EXECUTABLE(nl_sample_chatclient client.cpp kbhit.cpp kbhit.h)

ADD_EXECUTABLE(nl_sample_chatserver server.cpp)

ADD_DEFINITIONS(-DCHAT_DIR="\\"${NL_SHARE_PREFIX}/nl_sample_chat/\\"")


INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(nl_sample_chatclient ${PLATFORM_LINKFLAGS} nelmisc nelnet)
TARGET_LINK_LIBRARIES(nl_sample_chatserver ${PLATFORM_LINKFLAGS} nelmisc nelnet)
IF(WIN32)
  SET_TARGET_PROPERTIES(nl_sample_chatclient PROPERTIES 
    LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}"
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    PROJECT_LABEL "Samples, Net, Chat: Chat Client")
  SET_TARGET_PROPERTIES(nl_sample_chatserver PROPERTIES 
    LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS_RELEASE}"
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    PROJECT_LABEL "Samples, Net, Chat: Chat Server")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS nl_sample_chatclient nl_sample_chatserver RUNTIME DESTINATION bin COMPONENT samplesnet)
INSTALL(FILES chat_service.cfg client.cfg DESTINATION share/nel/nl_sample_chat COMPONENT samplesnet)
