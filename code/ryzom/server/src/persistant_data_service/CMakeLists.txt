FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_persistant_data_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_persistant_data_service
						ryzom_adminmodules
						ryzom_gameshare
						ryzom_pd
						${LIBXML2_LIBRARIES}
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(ryzom_persistant_data_service "Ryzom, Services: Persistent Data Service (PDS)")
NL_ADD_RUNTIME_FLAGS(ryzom_persistant_data_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_persistant_data_service RUNTIME DESTINATION sbin COMPONENT services)
