FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_mirror_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_mirror_service
					ryzom_adminmodules
					ryzom_gameshare
					${LIBXML2_LIBRARIES} 
					nelmisc
					nelnet
					nelgeorges)

NL_DEFAULT_PROPS(ryzom_mirror_service "Ryzom, Services: Mirror Service (MS)")
NL_ADD_RUNTIME_FLAGS(ryzom_mirror_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ryzom_mirror_service RUNTIME DESTINATION sbin COMPONENT services)
