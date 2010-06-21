FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(shard_unifier_service ${SRC})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(shard_unifier_service	admin_modules 
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
  SET_TARGET_PROPERTIES(shard_unifier_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Shard Unifier Service (SUS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS shard_unifier_service RUNTIME DESTINATION sbin COMPONENT services)


