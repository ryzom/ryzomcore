# Tests of the QUIC streams of the frontend service: the record framing and the stream classes of msg.xml
# (game_share), and what decides how much stream data each client gets (the library ryzom_frontend_streams, which the
# service links too).

# The simulation of the streams in made-up time (stream_sim.h): a library for the tests, and a program that prints
# what happens in its cases (ryzom_frontend_streams_sim --help).
add_library(ryzom_frontend_streams_simlib STATIC stream_sim.cpp stream_sim.h)
target_link_libraries(ryzom_frontend_streams_simlib PUBLIC Ryzom::frontend_streams Ryzom::gameshare NeL::misc)
target_compile_features(ryzom_frontend_streams_simlib PUBLIC cxx_std_17)

add_executable(ryzom_frontend_streams_sim stream_sim_main.cpp)
target_link_libraries(ryzom_frontend_streams_sim PRIVATE ryzom_frontend_streams_simlib)

add_test_executable(ryzom_frontend_streams_test
        test_context.cpp
        stream_record.test.cpp
        stream_scheduler.test.cpp
        stream_sim.test.cpp
        msg_xml_streams.test.cpp
)
target_link_libraries(ryzom_frontend_streams_test
        PRIVATE
        ryzom_frontend_streams_simlib
        Ryzom::frontend_streams
        Ryzom::gameshare
        NeL::misc
)
target_compile_definitions(ryzom_frontend_streams_test
        PRIVATE
        RYZOM_MSG_XML="${CMAKE_SOURCE_DIR}/ryzom/common/data_common/msg.xml"
        RYZOM_DATABASE_XML="${CMAKE_SOURCE_DIR}/ryzom/common/data_common/database.xml"
)
