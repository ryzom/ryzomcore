FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_mirror_service WIN32 ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_mirror_service PRIVATE ${RZ_SERVER_SRC_DIR})

TARGET_LINK_LIBRARIES(ryzom_mirror_service
					ryzom_adminmodules
					ryzom_gameshare
					nelmisc
					nelnet
					nelgeorges)

NL_DEFAULT_PROPS(ryzom_mirror_service "Ryzom, Services: Mirror Service (MS)")
NL_ADD_RUNTIME_FLAGS(ryzom_mirror_service)

INSTALL(TARGETS ryzom_mirror_service RUNTIME DESTINATION sbin COMPONENT services)
