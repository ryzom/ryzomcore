FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/bot_chat_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/bot_chat_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log_inline.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_pd.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ios_pd.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_pd_inline.h)

ADD_EXECUTABLE(ios_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ios_service	admin_modules 
					pd_lib
					ryzom_gameshare 
					server_share
					${PLATFORM_LINKFLAGS} 
					${LIBXML2_LIBRARIES} 
					${NELMISC_LIBRARY} 
					${NELNET_LIBRARY} 
					${NELPACS_LIBRARY}
					${NELGEORGES_LIBRARY} 
					${NELLIGO_LIBRARY})
IF(WIN32)
  SET_TARGET_PROPERTIES(ios_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Input Ouput Service (IOS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ios_service RUNTIME DESTINATION sbin COMPONENT services)


