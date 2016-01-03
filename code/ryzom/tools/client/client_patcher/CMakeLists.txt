FILE(GLOB SRC main.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/app_bundle_utils.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/user_agent.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/client_cfg.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/login_patch.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/login_xdelta.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h
)

# always enable custom patch server
ADD_DEFINITIONS(-DRZ_USE_CUSTOM_PATCH_SERVER)

ADD_EXECUTABLE(ryzom_client_patcher ${SRC})

INCLUDE_DIRECTORIES(
			${LIBXML2_INCLUDE_DIR}
			${CURL_INCLUDE_DIRS}
			${ZLIB_INCLUDE_DIR}
			${CMAKE_SOURCE_DIR}/ryzom/client/src
)

TARGET_LINK_LIBRARIES(ryzom_client_patcher
				nelmisc
				nelnet
				ryzom_gameshare
				ryzom_sevenzip
				${ZLIB_LIBRARIES}
				${CURL_LIBRARIES})

IF(APPLE)
  FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
  TARGET_LINK_LIBRARIES(ryzom_client_patcher ${FOUNDATION_LIBRARY})
ENDIF(APPLE)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} ${CURL_DEFINITIONS} -DRZ_NO_CLIENT -DNL_USE_SEVENZIP)

NL_DEFAULT_PROPS(ryzom_client_patcher "Ryzom, Tools: Ryzom Client Patcher")
NL_ADD_RUNTIME_FLAGS(ryzom_client_patcher)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ryzom_client_patcher ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp)
ENDIF(WITH_PCH)

INSTALL(TARGETS ryzom_client_patcher RUNTIME DESTINATION ${RYZOM_GAMES_PREFIX} COMPONENT client BUNDLE DESTINATION /Applications)

IF(WITH_RYZOM_CLIENT AND APPLE)
  IF(CODESIGN_ALLOCATE AND APPLE_CERTIFICATE)
    ADD_CUSTOM_COMMAND(TARGET ryzom_client_patcher POST_BUILD COMMAND CODESIGN_ALLOCATE=${CODESIGN_ALLOCATE} codesign -fs "${APPLE_CERTIFICATE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ryzom_client_patcher" COMMENT "Signing ryzom_client_patcher executable...")
  ENDIF()

  ADD_CUSTOM_COMMAND(TARGET ryzom_client_patcher POST_BUILD COMMAND cp -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ryzom_client_patcher ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MACOSX_BUNDLE_BUNDLE_NAME}.app/Contents/MacOS/RyzomClientPatcher)
ENDIF()
