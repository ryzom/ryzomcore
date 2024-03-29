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

INCLUDE_DIRECTORIES(${RZ_SERVER_SRC_DIR} ${MYSQL_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(ryzom_ios_service
					ryzom_adminmodules
					ryzom_pd
					ryzom_gameshare
					ryzom_servershare
					nelmisc
					nelnet
					nelpacs
					nelgeorges
					nelligo)

NL_DEFAULT_PROPS(ryzom_ios_service "Ryzom, Services: Input Output Service (IOS)")
NL_ADD_RUNTIME_FLAGS(ryzom_ios_service)

IF(WITH_PCH)
  target_precompile_headers(ryzom_ios_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS ryzom_ios_service RUNTIME DESTINATION sbin COMPONENT services)


