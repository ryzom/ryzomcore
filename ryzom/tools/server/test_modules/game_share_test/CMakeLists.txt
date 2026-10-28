add_test_executable(ryzom_server_test_module_interface
        test_module_interface.cpp
        test_module_itf.cpp
)

target_link_libraries(ryzom_server_test_module_interface
        PRIVATE
        NeL::misc
        NeL::net
)

add_test_executable(ryzom_game_share_scenario_text_parser
        scenario_text_parser_test.cpp
)

target_include_directories(ryzom_game_share_scenario_text_parser
        PRIVATE
        ${CMAKE_SOURCE_DIR}/ryzom/common/src
)

target_compile_definitions(ryzom_game_share_scenario_text_parser
        PRIVATE
        RYZOM_EXAMPLES_DIR="${CMAKE_SOURCE_DIR}/ryzom/client/examples"
)

target_link_libraries(ryzom_game_share_scenario_text_parser
        PRIVATE
        Ryzom::gameshare
        NeL::misc
)
