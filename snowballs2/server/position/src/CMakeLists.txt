FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_position_service WIN32 ${SRC})

TARGET_LINK_LIBRARIES(snowballs_position_service
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(snowballs_position_service "Snowballs, Services: Position Service")
NL_ADD_RUNTIME_FLAGS(snowballs_position_service)

INSTALL(TARGETS snowballs_position_service RUNTIME DESTINATION sbin COMPONENT services)
