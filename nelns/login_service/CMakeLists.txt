add_library(nelns_login_service STATIC
        nelns/login_service/connection_client.cpp
        nelns/login_service/connection_web.cpp
        nelns/login_service/connection_ws.cpp
        nelns/login_service/functions.cpp
        nelns/login_service/login_service.cpp
        nelns/login_service/mysql_helper.cpp
        nelns/login_service/mysql_persistence.cpp
        nelns/login_service/variables.cpp
)
add_library(nelns::ls ALIAS nelns_login_service)

target_compile_features(nelns_login_service
        PRIVATE
        cxx_lambdas
        cxx_return_type_deduction
)

target_include_directories(nelns_login_service PUBLIC ${ZLIB_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nelns_login_service
        MySQL::MySQL
        ${ZLIB_LIBRARIES}
        NeL::misc
        NeL::net
)

ADD_EXECUTABLE(login_service WIN32
        login_service.cpp
)

TARGET_LINK_LIBRARIES(login_service
        nelns::ls
)

NL_DEFAULT_PROPS(login_service "NelNS, Services: Login Service")
NL_ADD_RUNTIME_FLAGS(login_service)

INSTALL(TARGETS login_service RUNTIME DESTINATION sbin COMPONENT ls)
INSTALL(FILES login_service.cfg common.cfg DESTINATION ${NL_ETC_PREFIX}/nelns COMPONENT ls)

IF (WITH_NEL_TESTS OR WITH_TESTING)
    ADD_SUBDIRECTORY(tests)
ENDIF ()
