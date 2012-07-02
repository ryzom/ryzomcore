FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_collision_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(snowballs_collision_service
						${LIBXML2_LIBRARIES}
						nelmisc
						nelnet
						nelpacs
						nel3d)

NL_DEFAULT_PROPS(snowballs_collision_service "Snowballs, Services: Collision")
NL_ADD_RUNTIME_FLAGS(snowballs_collision_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS snowballs_collision_service RUNTIME DESTINATION sbin COMPONENT services)
