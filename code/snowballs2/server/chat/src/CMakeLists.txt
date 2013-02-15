FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_chat_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(snowballs_chat_service
						${LIBXML2_LIBRARIES}
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(snowballs_chat_service "Snowballs, Services: Chat")
NL_ADD_RUNTIME_FLAGS(snowballs_chat_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS snowballs_chat_service RUNTIME DESTINATION sbin COMPONENT services)
