FILE(GLOB SRC *.cpp *.h)

# What decides how much stream data each client gets (QUIC streams, protocol level ryzomcore/4.2). It knows neither
# MsQuic nor the service, and is a library of its own so that the tests and the simulation in tests/ link the very
# code the service runs.
SET(STREAMS_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/stream_queues.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/stream_queues.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stream_scheduler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/stream_scheduler.h)
LIST(REMOVE_ITEM SRC ${STREAMS_SRC})

ADD_LIBRARY(ryzom_frontend_streams STATIC ${STREAMS_SRC})
ADD_LIBRARY(Ryzom::frontend_streams ALIAS ryzom_frontend_streams)
TARGET_INCLUDE_DIRECTORIES(ryzom_frontend_streams PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(ryzom_frontend_streams PUBLIC Ryzom::gameshare NeL::misc)
NL_DEFAULT_PROPS(ryzom_frontend_streams "Ryzom, Library: Frontend Service Streams")

LIST(REMOVE_ITEM SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/pair_selector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pair_selector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/prioritizer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/prioritizer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/property_dispatcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/property_dispatcher.h)

SOURCE_GROUP("" FILES ${SRC})

ADD_EXECUTABLE(ryzom_frontend_service WIN32 ${SRC})

TARGET_LINK_LIBRARIES(ryzom_frontend_service
        PRIVATE
        Ryzom::frontend_streams
        Ryzom::adminmodules
        Ryzom::gameshare
        MySQL::MySQL
        NeL::misc
        NeL::net
        NeL::georges
        NeL::ligo
)

IF (WITH_MSQUIC)
    TARGET_LINK_LIBRARIES(ryzom_frontend_service PRIVATE msquic::msquic)
ENDIF ()

NL_DEFAULT_PROPS(ryzom_frontend_service "Ryzom, Services: Frontend Service (FS)")
NL_ADD_RUNTIME_FLAGS(ryzom_frontend_service)

IF (WITH_PCH)
    target_precompile_headers(ryzom_frontend_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF ()

INSTALL(TARGETS ryzom_frontend_service RUNTIME DESTINATION sbin COMPONENT services)

IF (WITH_NEL_TESTS OR WITH_TESTING)
    ADD_SUBDIRECTORY(tests)
ENDIF ()
