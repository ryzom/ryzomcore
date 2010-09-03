FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(tick_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(tick_service	admin_modules 
						ryzom_gameshare
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(tick_service "Ryzom, Services: Tick Service (TS)")
NL_ADD_RUNTIME_FLAGS(tick_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS tick_service RUNTIME DESTINATION sbin COMPONENT services)


