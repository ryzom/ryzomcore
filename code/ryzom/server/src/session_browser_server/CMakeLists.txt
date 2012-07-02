FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_session_browser_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_session_browser_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_servershare
						${LIBXML2_LIBRARIES} 
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARY}
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(ryzom_session_browser_service "Ryzom, Services: Session Browser Service (SBS)")
NL_ADD_RUNTIME_FLAGS(ryzom_session_browser_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_session_browser_service RUNTIME DESTINATION sbin COMPONENT services)
