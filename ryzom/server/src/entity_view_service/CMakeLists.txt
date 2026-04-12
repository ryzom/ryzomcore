FILE(GLOB SRC *.cpp *.h)

#LIST(REMOVE_ITEM SRC ${CMAKE_CURRENT_SOURCE_DIR}/service.cpp ${CMAKE_CURRENT_SOURCE_DIR}/service.h)

ADD_EXECUTABLE(ryzom_entity_view_service ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_entity_view_service PRIVATE ${RZ_SERVER_SRC_DIR} ${NEL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_entity_view_service
        PRIVATE
        Ryzom::adminmodules
        Ryzom::gameshare
        Ryzom::servershare
        Ryzom::aishare
        NeL::misc
        NeL::net
        NeL::georges
        NeL::ligo
)

NL_DEFAULT_PROPS(ryzom_entity_view_service "Ryzom, Services: Entity View Service (EVS)")
NL_ADD_RUNTIME_FLAGS(ryzom_entity_view_service)

INSTALL(TARGETS ryzom_entity_view_service RUNTIME DESTINATION sbin COMPONENT services)

