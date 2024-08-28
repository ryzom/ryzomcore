FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(nel_unit_test
        nel_unit_test.cpp
)

INCLUDE_DIRECTORIES(${CPPTEST_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(nel_unit_test ${CPPTEST_LIBRARIES} nelmisc nelnet nelligo)
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
        nelmisc
)
target_compile_definitions(nel_misc_config_file_test
        PUBLIC
        NEL_UNIT_BASE="${CMAKE_CURRENT_SOURCE_DIR}/"
)
