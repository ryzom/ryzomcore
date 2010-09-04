FILE(GLOB SRC *.cpp *.h
	${CMAKE_SOURCE_DIR}/client/src/client_cfg.cpp
	${CMAKE_SOURCE_DIR}/client/src/login_patch.cpp
	${CMAKE_SOURCE_DIR}/client/src/login_xdelta.cpp
)

SET(SEVENZIP_LIBRARY "ryzom_sevenzip")

ADD_EXECUTABLE(ryzom_client_patcher ${SRC})

INCLUDE_DIRECTORIES(
			${CMAKE_BINARY_DIR}
			${LIBXML2_INCLUDE_DIR}
			${CURL_INCLUDE_DIRS}
			${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_SOURCE_DIR}/ryzom/client/src
)

TARGET_LINK_LIBRARIES(ryzom_client_patcher 	${PLATFORM_LINKFLAGS}
				nelmisc
				nelnet
				ryzom_gameshare
				${CURL_LIBRARIES}
				${SEVENZIP_LIBRARY})

ADD_DEFINITIONS(-DRZ_NO_CLIENT -DNL_USE_SEVENZIP)

NL_DEFAULT_PROPS(ryzom_client_patcher "Ryzom, Tools: Ryzom Client Patcher")
NL_ADD_RUNTIME_FLAGS(ryzom_client_patcher)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_client_patcher ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_client_patcher RUNTIME DESTINATION games COMPONENT client BUNDLE DESTINATION /Applications)
