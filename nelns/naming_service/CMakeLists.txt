add_library(nelns_naming_service STATIC
        nelns/naming_service/functions.cpp
        nelns/naming_service/naming_service.cpp
        nelns/naming_service/service_instance_manager.cpp
        nelns/naming_service/variables.cpp
)
add_library(nelns::ns ALIAS nelns_naming_service)

target_include_directories(nelns_naming_service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(nelns_naming_service
        nelmisc
        nelnet
)

ADD_EXECUTABLE(naming_service WIN32
        naming_service.cpp
)

TARGET_LINK_LIBRARIES(naming_service
        nelns::ns
)

NL_DEFAULT_PROPS(naming_service "NeLNS, Services: Naming Service")
NL_ADD_RUNTIME_FLAGS(naming_service)

INSTALL(TARGETS naming_service RUNTIME DESTINATION sbin COMPONENT ns)
INSTALL(FILES naming_service.cfg common.cfg DESTINATION ${NL_ETC_PREFIX}/nelns COMPONENT ns)

IF (WITH_NEL_TESTS)
    ADD_SUBDIRECTORY(tests)
ENDIF ()
