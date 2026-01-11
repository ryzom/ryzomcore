FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_chat_service WIN32 ${SRC})

TARGET_LINK_LIBRARIES(snowballs_chat_service
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(snowballs_chat_service "Snowballs, Services: Chat")
NL_ADD_RUNTIME_FLAGS(snowballs_chat_service)

INSTALL(TARGETS snowballs_chat_service RUNTIME DESTINATION sbin COMPONENT services)
