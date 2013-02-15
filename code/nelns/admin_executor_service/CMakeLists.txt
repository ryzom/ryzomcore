FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(admin_executor_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(admin_executor_service
						nelmisc
						nelnet)

NL_DEFAULT_PROPS(admin_executor_service "NelNS, Services: Admin Executor Service")
NL_ADD_RUNTIME_FLAGS(admin_executor_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS admin_executor_service RUNTIME DESTINATION sbin COMPONENT aes)
INSTALL(FILES admin_executor_service.cfg common.cfg DESTINATION ${NL_ETC_PREFIX}/nelns COMPONENT aes)
