set(TEST_NAME "naming_service_test")

add_executable("${TEST_NAME}"
        naming_service.test.cpp
)
target_link_libraries("${TEST_NAME}"
        GTest::gtest_main
        GTest::gmock
        nelns::ns
)
add_test(
        NAME "${TEST_NAME}"
        COMMAND "${TEST_NAME}" "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/reports/junit-${TEST_NAME}.xml"
)


set(TEST_NAME "service_instance_manager_test")

add_executable("${TEST_NAME}"
        service_instance_manager.test.cpp
)
target_link_libraries("${TEST_NAME}"
        GTest::gtest_main
        GTest::gmock
        nelns::ns
)
add_test(
        NAME "${TEST_NAME}"
        COMMAND "${TEST_NAME}" "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/reports/junit-${TEST_NAME}.xml"
)
