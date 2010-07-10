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
			${CMAKE_CURRENT_SOURCE_DIR}/ios_interface.h)

ADD_EXECUTABLE(ai_service ${SRC})




INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/server/src ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${NEL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ai_service	admin_modules 
					ryzom_gameshare 
					server_share
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
	PROJECT_LABEL "Services: AI Service (AIS)")
ENDIF(WIN32)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ai_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ai_service RUNTIME DESTINATION sbin COMPONENT services)

