FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_backup_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR}  ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_backup_service
					ryzom_adminmodules
					ryzom_gameshare
					ryzom_servershare
					${LIBXML2_LIBRARIES}
					${ZLIB_LIBRARIES}
					nelmisc 
					nelnet
					nelgeorges
					nelligo)
					
NL_DEFAULT_PROPS(ryzom_backup_service "Ryzom, Services: Backup Service (BS)")
NL_ADD_RUNTIME_FLAGS(ryzom_backup_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_backup_service RUNTIME DESTINATION sbin COMPONENT services)
