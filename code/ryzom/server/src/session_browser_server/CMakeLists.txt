FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(session_browser_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(session_browser_service	admin_modules 
						ryzom_gameshare
						server_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARY}
						${NELMISC_LIBRARY} 
						${NELNET_LIBRARY}
						${NELGEORGES_LIBRARY})
IF(WIN32)
  SET_TARGET_PROPERTIES(session_browser_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Session Browser Service (SBS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS session_browser_service RUNTIME DESTINATION sbin COMPONENT services)


