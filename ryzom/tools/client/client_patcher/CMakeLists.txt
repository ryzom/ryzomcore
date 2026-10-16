FILE(GLOB SRC main.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/app_bundle_utils.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/user_agent.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/login_patch.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/login_xdelta.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.cpp
        ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h
)

ADD_EXECUTABLE(ryzom_client_patcher ${SRC})

TARGET_INCLUDE_DIRECTORIES(ryzom_client_patcher
        PRIVATE
        ${CMAKE_SOURCE_DIR}/ryzom/client/src
)

TARGET_LINK_LIBRARIES(ryzom_client_patcher
        PRIVATE
        NeL::misc
        NeL::web
        NeL::net
        ryzom_gameshare
        NeL::sevenzip
        CURL::libcurl
        ZLIB::ZLIB
)

IF (APPLE)
    TARGET_LINK_LIBRARIES(ryzom_client_patcher PRIVATE ${FOUNDATION_FRAMEWORK})
ENDIF ()

NL_DEFAULT_PROPS(ryzom_client_patcher "Ryzom, Tools: Ryzom Client Patcher")
NL_ADD_RUNTIME_FLAGS(ryzom_client_patcher)

IF (WITH_PCH)
    target_precompile_headers(ryzom_client_patcher PRIVATE ${CMAKE_SOURCE_DIR}/ryzom/client/src/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_client_patcher RUNTIME DESTINATION ${RYZOM_GAMES_PREFIX} COMPONENT client)
