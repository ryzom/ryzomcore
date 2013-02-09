FILE(GLOB SRC *.cpp *.h)

LIST(REMOVE_ITEM SRC
			${CMAKE_CURRENT_SOURCE_DIR}/bot_chat_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/bot_chat_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_chat_log_inline.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_pd.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/ios_pd.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios_pd_inline.h)

ADD_EXECUTABLE(ryzom_ios_service WIN32 ${SRC})

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${LIBXML2_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(ryzom_ios_service
					ryzom_adminmodules
					ryzom_pd
					ryzom_gameshare
					ryzom_servershare
					${LIBXML2_LIBRARIES}
					nelmisc
					nelnet
					nelpacs
					nelgeorges
					nelligo)

NL_DEFAULT_PROPS(ryzom_ios_service "Ryzom, Services: Input Output Service (IOS)")
NL_ADD_RUNTIME_FLAGS(ryzom_ios_service)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_ios_service ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_ios_service RUNTIME DESTINATION sbin COMPONENT services)


