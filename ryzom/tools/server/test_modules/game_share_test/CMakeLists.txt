add_test_executable(ryzom_server_test_module_interface
        test_module_interface.cpp
        test_module_itf.cpp
)

target_link_libraries(ryzom_server_test_module_interface
        PRIVATE
        NeL::misc
        NeL::net
)
