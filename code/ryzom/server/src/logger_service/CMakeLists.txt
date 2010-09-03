FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(logger_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(logger_service	admin_modules 
						ryzom_gameshare
						server_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						nelmisc 
						nelnet)

NL_DEFAULT_PROPS(logger_service "Ryzom, Services: Logger Service (LGS)")
NL_ADD_RUNTIME_FLAGS(logger_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS logger_service RUNTIME DESTINATION sbin COMPONENT services)


