FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/pair_selector.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/pair_selector.h
			${CMAKE_CURRENT_SOURCE_DIR}/prioritizer.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/prioritizer.h
			${CMAKE_CURRENT_SOURCE_DIR}/property_dispatcher.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/property_dispatcher.h)

ADD_EXECUTABLE(ryzom_frontend_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_frontend_service
						ryzom_adminmodules
						ryzom_gameshare
						${LIBXML2_LIBRARIES}
						${MYSQL_LIBRARIES}
						${ZLIB_LIBRARIES}
						nelmisc
						nelnet
						nelgeorges
						nelligo)

NL_DEFAULT_PROPS(ryzom_frontend_service "Ryzom, Services: Frontend Service (FS)")
NL_ADD_RUNTIME_FLAGS(ryzom_frontend_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_frontend_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_frontend_service RUNTIME DESTINATION sbin COMPONENT services)
