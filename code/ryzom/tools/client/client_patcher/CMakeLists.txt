FILE(GLOB SRC main.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/client_cfg.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/login_patch.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/login_xdelta.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h
)

ADD_EXECUTABLE(ryzom_client_patcher ${SRC})

INCLUDE_DIRECTORIES(
			${CURL_INCLUDE_DIRS}
			${CMAKE_SOURCE_DIR}/ryzom/client/src
)

TARGET_LINK_LIBRARIES(ryzom_client_patcher 	${PLATFORM_LINKFLAGS}
				nelmisc
				nelnet
				ryzom_gameshare
				ryzom_sevenzip
				${CURL_LIBRARIES})

ADD_DEFINITIONS(${CURL_DEFINITIONS} -DRZ_NO_CLIENT -DNL_USE_SEVENZIP)

NL_DEFAULT_PROPS(ryzom_client_patcher "Ryzom, Tools: Ryzom Client Patcher")
NL_ADD_RUNTIME_FLAGS(ryzom_client_patcher)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_client_patcher ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_client_patcher RUNTIME DESTINATION games COMPONENT client BUNDLE DESTINATION /Applications)
