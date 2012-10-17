FILE(GLOB SRC *.cpp *.h)

#LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/ai_entity_id.cpp))

ADD_EXECUTABLE(ai_data_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ai_data_service 	admin_modules 
					ryzom_gameshare 
					server_share
					ai_share
					${LIBXML2_LIBRARIES} 
					${MYSQL_LIBRARIES} 
					${ZLIB_LIBRARIES} 
					nelmisc
					nelnet
					nelgeorges
					nelpacs
					nelligo)

NL_DEFAULT_PROPS(ai_data_service "Ryzom, Services: AI Data Service (AIDS)")
NL_ADD_RUNTIME_FLAGS(ai_data_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ai_data_service RUNTIME DESTINATION sbin COMPONENT services)

