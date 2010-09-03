FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(backup_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR}  ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(backup_service	admin_modules 
					ryzom_gameshare 
					server_share
					${PLATFORM_LINKFLAGS} 
					${LIBXML2_LIBRARIES} 
					${ZLIB_LIBRARIES} 
					nelmisc 
					nelnet
					nelgeorges
					nelligo)
					
NL_DEFAULT_PROPS(backup_service "Ryzom, Services: Backup Service (BS)")
NL_ADD_RUNTIME_FLAGS(backup_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS backup_service RUNTIME DESTINATION sbin COMPONENT services)

