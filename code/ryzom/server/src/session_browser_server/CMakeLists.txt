FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(session_browser_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(session_browser_service	admin_modules 
						ryzom_gameshare
						server_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARY}
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(session_browser_service "Ryzom, Services: Session Browser Service (SBS)")
NL_ADD_RUNTIME_FLAGS(session_browser_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS session_browser_service RUNTIME DESTINATION sbin COMPONENT services)


