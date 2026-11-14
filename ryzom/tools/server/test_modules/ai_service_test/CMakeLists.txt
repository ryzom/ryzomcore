# The script compiler and the script machine of the AI service, built from the sources of the
# service. What they use of the rest of the service is in the test.
add_test_executable(ryzom_ai_service_script
        script_test.cpp
        ${RZ_SERVER_SRC_DIR}/ai_service/script_parser_lex.cpp
        ${RZ_SERVER_SRC_DIR}/ai_service/script_parser_yacc.cpp
        ${RZ_SERVER_SRC_DIR}/ai_service/script_vm.cpp
)

target_include_directories(ryzom_ai_service_script
        PRIVATE
        ${RZ_SERVER_SRC_DIR}/ai_service
        ${RZ_SERVER_SRC_DIR}
        ${CMAKE_SOURCE_DIR}/ryzom/common/src
)

target_link_libraries(ryzom_ai_service_script
        PRIVATE
        Ryzom::gameshare
        NeL::misc
        NeL::net
        NeL::ligo
)
