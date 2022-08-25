FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_welcome_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_welcome_service
						ryzom_adminmodules
						ryzom_gameshare
						nelmisc 
						nelnet)

NL_DEFAULT_PROPS(ryzom_welcome_service "Ryzom, Services: Ryzom Welcome Service (WS)")
NL_ADD_RUNTIME_FLAGS(ryzom_welcome_service)

INSTALL(TARGETS ryzom_welcome_service RUNTIME DESTINATION sbin COMPONENT services)


