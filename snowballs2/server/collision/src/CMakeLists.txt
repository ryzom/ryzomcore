FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(snowballs_collision_service WIN32 ${SRC})

TARGET_LINK_LIBRARIES(snowballs_collision_service
						NeL::misc
						NeL::net
						NeL::pacs
						NeL::3d)

NL_DEFAULT_PROPS(snowballs_collision_service "Snowballs, Services: Collision")
NL_ADD_RUNTIME_FLAGS(snowballs_collision_service)

INSTALL(TARGETS snowballs_collision_service RUNTIME DESTINATION sbin COMPONENT services)
