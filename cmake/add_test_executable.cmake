add_custom_target(unit_test)

function(add_test_executable name)
    add_executable(${ARGV})
    target_link_libraries("${name}"
            GTest::gtest_main
            GTest::gmock
    )

    add_test(NAME "${name}" COMMAND "${name}")

    add_dependencies(unit_test "${name}")
endfunction()
