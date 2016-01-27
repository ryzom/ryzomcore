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
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_monitor_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_monitor_service RUNTIME DESTINATION sbin COMPONENT services)


