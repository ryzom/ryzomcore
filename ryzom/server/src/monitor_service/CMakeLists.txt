FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(ryzom_monitor_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_monitor_service
						ryzom_gameshare
						${MYSQL_LIBRARIES}
						nelmisc 
						nelnet
						nelgeorges)

NL_DEFAULT_PROPS(ryzom_monitor_service "Ryzom, Services: Monitor Service (MOS)")
NL_ADD_RUNTIME_FLAGS(ryzom_monitor_service)

IF(WITH_PCH)
  target_precompile_headers(ryzom_monitor_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS ryzom_monitor_service RUNTIME DESTINATION sbin COMPONENT services)


