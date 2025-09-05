FILE(GLOB SRC *.cpp *.h)

#LIST(REMOVE_ITEM SRC	${CMAKE_CURRENT_SOURCE_DIR}/ai_entity_id.cpp))

ADD_EXECUTABLE(ai_data_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ai_data_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ai_data_service
		PRIVATE
        admin_modules
        ryzom_gameshare
        server_share
        ai_share
        NeL::misc
        NeL::net
        NeL::georges
        NeL::pacs
        NeL::ligo
)

NL_DEFAULT_PROPS(ai_data_service "Ryzom, Services: AI Data Service (AIDS)")
NL_ADD_RUNTIME_FLAGS(ai_data_service)

INSTALL(TARGETS ai_data_service RUNTIME DESTINATION sbin COMPONENT services)

