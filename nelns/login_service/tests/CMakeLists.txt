add_test_executable(nelns_login_service_it
        login_service.it.cpp
)
target_link_libraries(nelns_login_service_it
        PRIVATE
        nelns::ls
)
