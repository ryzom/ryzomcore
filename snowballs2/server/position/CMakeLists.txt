ADD_EXECUTABLE(snowballs_position_service WIN32)

target_sources(snowballs_position_service
        PRIVATE
        snowballs/position/main.cpp
        snowballs/position/physics.cpp
)
target_sources(snowballs_position_service
        PUBLIC
        FILE_SET HEADERS
        FILES
        snowballs/position/physics.h
)

TARGET_LINK_LIBRARIES(snowballs_position_service
        NeL::misc
        NeL::net)

NL_DEFAULT_PROPS(snowballs_position_service "Snowballs, Services: Position Service")
NL_ADD_RUNTIME_FLAGS(snowballs_position_service)

INSTALL(TARGETS snowballs_position_service RUNTIME DESTINATION sbin COMPONENT services)
INSTALL(FILES position_service.cfg DESTINATION etc/snowballs COMPONENT services)

ADD_SUBDIRECTORY(tests)
