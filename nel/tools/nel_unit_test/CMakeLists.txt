if (WITH_GEORGES)
    add_test_executable(nel_georges_test
            ut_georges_form.test.cpp
    )
    target_link_libraries(nel_georges_test
            PRIVATE
            NeL::georges
    )
endif ()

add_test_executable(nel_ligo_test
        ut_ligo_primitive.test.cpp
)
target_link_libraries(nel_ligo_test
        PRIVATE
        NeL::ligo
)

add_test_executable(nel_misc_test
        ut_misc_base64.test.cpp
        ut_misc_bitmap.test.cpp
        ut_misc_co_task.test.cpp
        ut_misc_command.test.cpp
        ut_misc_common.test.cpp
        ut_misc_debug.test.cpp
        ut_misc_dynlibload.test.cpp
        ut_misc_file.test.cpp
        ut_misc_config_file.test.cpp
        ut_misc_pack_file.test.cpp
        ut_misc_path_safety.test.cpp
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

# The Vorbis targets are the ones found for the directory of nel. With Hunter they belong to the directory of the sound
# library and cannot be used from here: the decoder tests are then left out.
if (WITH_SOUND AND TARGET Vorbis::vorbisfile AND TARGET Vorbis::vorbis)
    add_test_executable(nel_sound_test
            ut_sound_audio_decoder.test.cpp
    )
    target_link_libraries(nel_sound_test
            PRIVATE
            NeL::misc
            NeL::sound
            Vorbis::vorbisfile
            Vorbis::vorbis
    )
    target_include_directories(nel_sound_test
            PRIVATE
            ${OGG_INCLUDE_DIR}
    )
    # The tests encode their own Ogg Vorbis streams where the encoder library is installed, and are left out elsewhere.
    find_library(VORBISENC_LIBRARY NAMES vorbisenc libvorbisenc)
    if (VORBISENC_LIBRARY)
        target_compile_definitions(nel_sound_test
                PRIVATE
                NL_UT_HAVE_VORBISENC
        )
        target_link_libraries(nel_sound_test
                PRIVATE
                ${VORBISENC_LIBRARY}
                ${OGG_LIBRARY}
        )
    else ()
        message(STATUS "nel_sound_test: no vorbisenc library, the tests that encode a stream are left out")
    endif ()
    # With ffmpeg, IAudioDecoder creates the ffmpeg decoder for every file: the test of what it creates for a file
    # that is not Ogg Vorbis is left out, the tests of the Vorbis decoder itself stay.
    if (FFMPEG_FOUND)
        target_compile_definitions(nel_sound_test
                PRIVATE
                NL_UT_SOUND_FFMPEG
        )
    endif ()
    # A read loop of a decoder that does not end must fail the test, not hang the run.
    set_tests_properties(nel_sound_test PROPERTIES TIMEOUT 120)
elseif (WITH_SOUND)
    message(STATUS "nel_sound_test: left out, the Vorbis targets are not visible from the unit test directory")
endif ()

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