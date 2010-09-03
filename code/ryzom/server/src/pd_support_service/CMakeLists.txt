FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(pd_support_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(pd_support_service	admin_modules 
						ryzom_gameshare
						server_share
						${PLATFORM_LINKFLAGS} 
						${LIBXML2_LIBRARIES} 
						nelmisc 
						nelnet)

NL_DEFAULT_PROPS(pd_support_service "Ryzom, Services: PD Support Service (PDSS)")
NL_ADD_RUNTIME_FLAGS(pd_support_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS pd_support_service RUNTIME DESTINATION sbin COMPONENT services)


