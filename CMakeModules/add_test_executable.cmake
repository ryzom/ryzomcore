function(add_test_executable name)
    add_executable(${ARGV})
    target_link_libraries("${name}"
            GTest::gtest_main
            GTest::gmock
    )
    add_test(NAME "${name}" COMMAND "${name}")
endfunction()
