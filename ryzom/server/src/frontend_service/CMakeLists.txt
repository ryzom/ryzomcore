FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/pair_selector.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/pair_selector.h
			${CMAKE_CURRENT_SOURCE_DIR}/prioritizer.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/prioritizer.h
			${CMAKE_CURRENT_SOURCE_DIR}/property_dispatcher.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/property_dispatcher.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(ryzom_frontend_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_frontend_service
						ryzom_adminmodules
						ryzom_gameshare
						${MYSQL_LIBRARIES}
						nelmisc
						nelnet
						nelgeorges
						nelligo)

IF(WITH_MSQUIC)
  INCLUDE_DIRECTORIES(${msquic_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(ryzom_frontend_service ${msquic_LIBRARIES})
ENDIF()

NL_DEFAULT_PROPS(ryzom_frontend_service "Ryzom, Services: Frontend Service (FS)")
NL_ADD_RUNTIME_FLAGS(ryzom_frontend_service)

IF(WITH_PCH)
  target_precompile_headers(ryzom_frontend_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS ryzom_frontend_service RUNTIME DESTINATION sbin COMPONENT services)
