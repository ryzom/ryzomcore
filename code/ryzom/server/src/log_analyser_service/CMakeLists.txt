FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(log_analyser_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(log_analyser_service	admin_modules
						ryzom_gameshare
						pd_lib
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(log_analyser_service "Ryzom, Services: Log Analyzer Service (LAS)")
NL_ADD_RUNTIME_FLAGS(log_analyser_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS log_analyser_service RUNTIME DESTINATION sbin COMPONENT services)


