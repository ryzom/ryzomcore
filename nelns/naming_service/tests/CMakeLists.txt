add_test_executable(nelns_do_allocate_port_test
        do_allocate_port.test.cpp
)
target_link_libraries(nelns_do_allocate_port_test
        nelns::ns
)


add_test_executable(nelns_naming_service_test
        naming_service.test.cpp
)
target_link_libraries(nelns_naming_service_test
        nelns::ns
)


add_test_executable(nelns_service_instance_manager_test
        service_instance_manager.test.cpp
)
target_link_libraries(nelns_service_instance_manager_test
        nelns::ns
)

add_test_executable(nelns_naming_service_it
        naming_service.it.cpp
)
target_link_libraries(nelns_naming_service_it
        nelns::ns
)
