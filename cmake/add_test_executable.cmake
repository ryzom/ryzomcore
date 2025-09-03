if (NOT TARGET unit_test)
    add_custom_target(unit_test)
endif()

function(add_test_executable name)
    add_executable(${ARGV})
    target_link_libraries("${name}"
            PRIVATE
            GTest::gtest_main
            GTest::gmock
    )

    add_test(NAME "${name}" COMMAND "${name}")
    target_compile_features("${name}" PRIVATE cxx_std_17)

    add_dependencies(unit_test "${name}")
endfunction()
