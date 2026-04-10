FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_monitor_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_monitor_service PRIVATE ${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_monitor_service
        PRIVATE
        ryzom_gameshare
        MySQL::MySQL
        NeL::misc
        NeL::net
        NeL::georges
)

NL_DEFAULT_PROPS(ryzom_monitor_service "Ryzom, Services: Monitor Service (MOS)")
NL_ADD_RUNTIME_FLAGS(ryzom_monitor_service)

IF (WITH_PCH)
    target_precompile_headers(ryzom_monitor_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_monitor_service RUNTIME DESTINATION sbin COMPONENT services)


