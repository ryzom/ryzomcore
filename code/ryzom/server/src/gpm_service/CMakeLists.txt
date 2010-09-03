FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(gpm_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(gpm_service	admin_modules 
					ryzom_gameshare 
					server_share
					${PLATFORM_LINKFLAGS} 
					${LIBXML2_LIBRARIES} 
					nelmisc 
					nelnet
					nelpacs
					nelgeorges
					nelligo)

NL_DEFAULT_PROPS(gpm_service "Ryzom, Services: Global Position Manager Service (GPMS)")
NL_ADD_RUNTIME_FLAGS(gpm_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS gpm_service RUNTIME DESTINATION sbin COMPONENT services)

