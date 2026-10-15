FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_logger_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_logger_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_servershare
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(ryzom_logger_service "Ryzom, Services: Logger Service (LGS)")
NL_ADD_RUNTIME_FLAGS(ryzom_logger_service)

INSTALL(TARGETS ryzom_logger_service RUNTIME DESTINATION sbin COMPONENT services)
