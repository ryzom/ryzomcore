FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(naming_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(naming_service
						${LIBXML2_LIBRARIES}
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(naming_service "NeLNS, Services: Naming Service")
NL_ADD_RUNTIME_FLAGS(naming_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS naming_service RUNTIME DESTINATION sbin COMPONENT ns)
INSTALL(FILES naming_service.cfg common.cfg DESTINATION ${NL_ETC_PREFIX}/nelns COMPONENT ns)
