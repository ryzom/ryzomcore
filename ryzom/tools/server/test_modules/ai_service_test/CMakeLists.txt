# The script compiler and the script machine of the AI service, built from the sources of the
# service. What they use of the rest of the service is in the test.
# (The headers of the service bring in the map of ai_share with its inline functions, which a
# build without optimisation keeps: they need its library.)
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
        Ryzom::aishare
        Ryzom::gameshare
        NeL::misc
        NeL::net
        NeL::ligo
)
