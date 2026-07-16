FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_backup_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_backup_service PRIVATE ${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_backup_service
        PRIVATE
        Ryzom::adminmodules
        Ryzom::gameshare
        Ryzom::servershare
        NeL::misc
        NeL::net
        NeL::georges
        NeL::ligo
)

NL_DEFAULT_PROPS(ryzom_backup_service "Ryzom, Services: Backup Service (BS)")
NL_ADD_RUNTIME_FLAGS(ryzom_backup_service)

INSTALL(TARGETS ryzom_backup_service RUNTIME DESTINATION sbin COMPONENT services)
