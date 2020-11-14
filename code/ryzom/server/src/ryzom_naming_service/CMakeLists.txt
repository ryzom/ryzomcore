FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_naming_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_naming_service
						ryzom_adminmodules
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(ryzom_naming_service "Ryzom, Services: Ryzom Naming Service (NS)")
NL_ADD_RUNTIME_FLAGS(ryzom_naming_service)

INSTALL(TARGETS ryzom_naming_service RUNTIME DESTINATION sbin COMPONENT services)


