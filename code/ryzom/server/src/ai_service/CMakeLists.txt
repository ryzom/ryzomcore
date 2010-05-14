FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/ai_entity_id.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ai_entity_id.h
			${CMAKE_CURRENT_SOURCE_DIR}/ai_entity_id_inline.h
			${CMAKE_CURRENT_SOURCE_DIR}/ai_world_map.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ai_world_map.h
			${CMAKE_CURRENT_SOURCE_DIR}/combat_move.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/combat_move.h
			${CMAKE_CURRENT_SOURCE_DIR}/gpms_interface.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/gpms_interface.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_interface.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ios_interface.h
			${CMAKE_CURRENT_SOURCE_DIR}/Backup\ 1\ of\ ai_profile_fauna.h
			${CMAKE_CURRENT_SOURCE_DIR}/Backup\ 1\ of\ profile.h
			${CMAKE_CURRENT_SOURCE_DIR}/Backup\ 1\ of\ stdpch.h)

ADD_EXECUTABLE(ai_service ${SRC})




INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ai_service	admin_modules 
					game_share 
					ai_share
					${PLATFORM_LINKFLAGS} 
					${LIBXML2_LIBRARIES} 
					${MYSQL_LIBRARIES} 
					${ZLIB_LIBRARIES} 
					${NELMISC_LIBRARY} 
					${NELNET_LIBRARY} 
					${NELGEORGES_LIBRARY} 
					${NELLIGO_LIBRARY})
IF(WIN32)
  SET_TARGET_PROPERTIES(ai_service PROPERTIES 
	LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:WINDOWS"
	PROJECT_LABEL "Services: Frontend Service (FS)")
ENDIF(WIN32)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ai_service RUNTIME DESTINATION sbin COMPONENT services)

