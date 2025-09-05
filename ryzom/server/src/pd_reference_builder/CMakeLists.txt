FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_reference_builder_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_reference_builder_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_reference_builder_service
        PRIVATE
        ryzom_gameshare
        ryzom_pd
        NeL::misc
        NeL::net
        NeL::georges
)

NL_DEFAULT_PROPS(ryzom_reference_builder_service "Ryzom, Services: Reference Builder Service (RBS)")
NL_ADD_RUNTIME_FLAGS(ryzom_reference_builder_service)

INSTALL(TARGETS ryzom_reference_builder_service RUNTIME DESTINATION sbin COMPONENT services)
