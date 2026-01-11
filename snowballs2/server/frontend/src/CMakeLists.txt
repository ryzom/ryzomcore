FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_frontend_service WIN32 ${SRC})

TARGET_LINK_LIBRARIES(snowballs_frontend_service
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(snowballs_frontend_service "Snowballs, Services: Frontend Service")
NL_ADD_RUNTIME_FLAGS(snowballs_frontend_service)

INSTALL(TARGETS snowballs_frontend_service RUNTIME DESTINATION sbin COMPONENT services)
