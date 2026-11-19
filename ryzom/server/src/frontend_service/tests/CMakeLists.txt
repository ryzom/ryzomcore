# Tests of the QUIC streams of the frontend service: the record framing and the stream classes of msg.xml
# (game_share), and what decides how much stream data each client gets (the library ryzom_frontend_streams, which the
# service links too).

add_test_executable(ryzom_frontend_streams_test
        test_context.cpp
        stream_record.test.cpp
        stream_scheduler.test.cpp
        msg_xml_streams.test.cpp
)
target_link_libraries(ryzom_frontend_streams_test
        PRIVATE
        Ryzom::frontend_streams
        Ryzom::gameshare
        NeL::misc
)
target_compile_definitions(ryzom_frontend_streams_test
        PRIVATE
        RYZOM_MSG_XML="${CMAKE_SOURCE_DIR}/ryzom/common/data_common/msg.xml"
)
