FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_reference_builder_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_reference_builder_service
						ryzom_gameshare
						ryzom_pd
						nelmisc
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(ryzom_reference_builder_service "Ryzom, Services: Reference Builder Service (RBS)")
NL_ADD_RUNTIME_FLAGS(ryzom_reference_builder_service)

INSTALL(TARGETS ryzom_reference_builder_service RUNTIME DESTINATION sbin COMPONENT services)
