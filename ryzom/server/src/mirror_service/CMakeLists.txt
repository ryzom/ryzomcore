FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_mirror_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_mirror_service
					ryzom_adminmodules
					${RYZOM_GAMESHARE_LIBRARIES}
					${NELMISC_LIBRARIES}
					${NELNET_LIBRARIES}
					${NELGEORGES_LIBRARIES})

NL_DEFAULT_PROPS(ryzom_mirror_service "Ryzom, Services: Mirror Service (MS)")
NL_ADD_RUNTIME_FLAGS(ryzom_mirror_service)

INSTALL(TARGETS ryzom_mirror_service RUNTIME DESTINATION sbin COMPONENT services)
