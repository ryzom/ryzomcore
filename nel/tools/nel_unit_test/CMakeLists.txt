ADD_EXECUTABLE(nel_unit_test
        nel_unit_test.cpp
)
ADD_TEST(NAME nel_unit_test COMMAND nel_unit_test)

TARGET_LINK_LIBRARIES(nel_unit_test
        PRIVATE
        CppTest::CppTest
        NeL::misc
        NeL::net
        NeL::ligo
)

NL_DEFAULT_PROPS(nel_unit_test "Unit Tests")
NL_ADD_RUNTIME_FLAGS(nel_unit_test)

target_compile_definitions(nel_unit_test
        PUBLIC
        NEL_UNIT_BASE="${PROJECT_SOURCE_DIR}/tools/nel_unit_test/"
)

INSTALL(TARGETS nel_unit_test RUNTIME DESTINATION ${NL_BIN_PREFIX})


add_test_executable(nel_misc_config_file_test
        ut_misc_config_file.test.cpp
)
target_link_libraries(nel_misc_config_file_test
        PRIVATE
        NeL::misc
)
target_compile_definitions(nel_misc_config_file_test
        PUBLIC
        NEL_UNIT_BASE="${CMAKE_CURRENT_SOURCE_DIR}/"
)


add_test_executable(nel_net_module_test
        ut_net_module.test.cpp
)
target_link_libraries(nel_net_module_test
        PRIVATE
        NeL::misc
        NeL::net
)
