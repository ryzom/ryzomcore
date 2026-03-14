add_test_executable(nel_ligo_test
        ut_ligo_primitive.test.cpp
)
target_link_libraries(nel_ligo_test
        PRIVATE
        NeL::ligo
)

add_test_executable(nel_misc_test
        ut_misc_base64.test.cpp
        ut_misc_co_task.test.cpp
        ut_misc_command.test.cpp
        ut_misc_common.test.cpp
        ut_misc_debug.test.cpp
        ut_misc_dynlibload.test.cpp
        ut_misc_file.test.cpp
        ut_misc_config_file.test.cpp
        ut_misc_pack_file.test.cpp
        ut_misc_singleton.test.cpp
        ut_misc_sstring.test.cpp
        ut_misc_stream.test.cpp
        ut_misc_string_common.test.cpp
        ut_misc_types.test.cpp
        ut_misc_variable.test.cpp
)
target_link_libraries(nel_misc_test
        PRIVATE
        NeL::misc
)
target_compile_definitions(nel_misc_test
        PUBLIC
        NEL_UNIT_BASE="${CMAKE_CURRENT_SOURCE_DIR}/"
)


add_test_executable(nel_net_test
        ut_net_address.test.cpp
        ut_net_layer3.test.cpp
        ut_net_message.test.cpp
        ut_net_module.test.cpp
)
target_link_libraries(nel_net_test
        PRIVATE
        NeL::misc
        NeL::net
)


add_test_executable(nel_web_http_client_it
        ut_web_http_client.it.cpp
)
target_link_libraries(nel_web_http_client_it
        PRIVATE
        NeL::web
)

if (WITH_DRIVER_OPENAL)
    add_test_executable(nel_sound_openal_it
            ut_sound_openal.it.cpp
    )
    target_link_libraries(nel_sound_openal_it
            PRIVATE
            NeL::misc
            NeL::sound
    )
    add_dependencies(nel_sound_openal_it
            NeL::sound::OpenAL
    )
endif ()