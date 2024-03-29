FILE(GLOB SRC main.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/app_bundle_utils.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/user_agent.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/login_patch.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/login_xdelta.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h
)

# always enable custom patch server
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

ADD_EXECUTABLE(ryzom_client_patcher ${SRC})

INCLUDE_DIRECTORIES(
			${LIBXML2_INCLUDE_DIR}
			${CURL_INCLUDE_DIRS}
			${ZLIB_INCLUDE_DIR}
			${CMAKE_SOURCE_DIR}/ryzom/client/src
)

TARGET_LINK_LIBRARIES(ryzom_client_patcher
				nelmisc
				nelweb
				nelnet
				ryzom_gameshare
				nel_sevenzip
				${CURL_LIBRARIES})

IF(APPLE)
  TARGET_LINK_LIBRARIES(ryzom_client_patcher ${FOUNDATION_FRAMEWORK})
ENDIF()

ADD_DEFINITIONS(${CURL_DEFINITIONS})

NL_DEFAULT_PROPS(ryzom_client_patcher "Ryzom, Tools: Ryzom Client Patcher")
NL_ADD_RUNTIME_FLAGS(ryzom_client_patcher)

IF(WITH_PCH)
  target_precompile_headers(ryzom_client_patcher PRIVATE ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h)
ENDIF()

INSTALL(TARGETS ryzom_client_patcher RUNTIME DESTINATION ${RYZOM_GAMES_PREFIX} COMPONENT client)
