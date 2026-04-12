FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_session_browser_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_session_browser_service PRIVATE ${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_session_browser_service
        PRIVATE
        Ryzom::adminmodules
        Ryzom::gameshare
        Ryzom::servershare
        NeL::misc
        NeL::net
        NeL::georges
)

NL_DEFAULT_PROPS(ryzom_session_browser_service "Ryzom, Services: Session Browser Service (SBS)")
NL_ADD_RUNTIME_FLAGS(ryzom_session_browser_service)

INSTALL(TARGETS ryzom_session_browser_service RUNTIME DESTINATION sbin COMPONENT services)
