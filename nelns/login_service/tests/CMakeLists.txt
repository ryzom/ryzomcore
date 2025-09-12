add_test_executable(nelns_login_service_client_connection_it
        login_service_client_connection.it.cpp
)
target_link_libraries(nelns_login_service_client_connection_it
        PRIVATE
        nelns::ls
)
