FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_log_analyser_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_log_analyser_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_pd
						nelmisc
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(ryzom_log_analyser_service "Ryzom, Services: Log Analyzer Service (LAS)")
NL_ADD_RUNTIME_FLAGS(ryzom_log_analyser_service)

INSTALL(TARGETS ryzom_log_analyser_service RUNTIME DESTINATION sbin COMPONENT services)
