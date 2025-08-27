if (NOT TARGET unit_test)
    add_custom_target(unit_test)
endif()

function(add_test_executable name)
    add_executable(${ARGV})
    target_link_libraries("${name}"
            GTest::gtest_main
            GTest::gmock
    )
    target_compile_features("${name}" PRIVATE cxx_std_17)

    add_test(NAME "${name}" COMMAND "${name}")

    add_dependencies(unit_test "${name}")
endfunction()
