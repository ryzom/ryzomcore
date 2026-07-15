FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_tick_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_tick_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_tick_service
        PRIVATE
        Ryzom::adminmodules
        Ryzom::gameshare
        NeL::misc
        NeL::net
        NeL::georges
)

NL_DEFAULT_PROPS(ryzom_tick_service "Ryzom, Services: Tick Service (TS)")
NL_ADD_RUNTIME_FLAGS(ryzom_tick_service)

INSTALL(TARGETS ryzom_tick_service RUNTIME DESTINATION sbin COMPONENT services)
