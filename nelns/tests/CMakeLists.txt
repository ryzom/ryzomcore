set(TEST_NAME "naming_service_test")

add_executable("${TEST_NAME}"
        naming_service.test.cpp
)
target_link_libraries("${TEST_NAME}"
        GTest::gtest_main
        nelmisc
        nelnet
)
gtest_discover_tests("${TEST_NAME}"
        EXTRA_ARGS "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/reports/junit-${TEST_NAME}.xml"
)

