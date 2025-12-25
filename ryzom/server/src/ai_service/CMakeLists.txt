FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/ai_entity_id.cpp
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

ADD_EXECUTABLE(ryzom_ai_service WIN32 ${SRC})

IF (APPLE)
    target_compile_options(ryzom_ai_service "-ftemplate-depth=100")
ENDIF ()

TARGET_INCLUDE_DIRECTORIES(ryzom_ai_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_ai_service
        PRIVATE
        ryzom_adminmodules
        ryzom_gameshare
        ryzom_servershare
        ryzom_aishare
        NeL::misc
        NeL::net
        NeL::georges
        NeL::ligo
)

NL_DEFAULT_PROPS(ryzom_ai_service "Ryzom, Services: AI Service (AIS)")
NL_ADD_RUNTIME_FLAGS(ryzom_ai_service)

IF (WITH_PCH)
    target_precompile_headers(ryzom_ai_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_ai_service RUNTIME DESTINATION sbin COMPONENT services)

