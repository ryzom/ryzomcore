include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(nelns_naming_service
        nelns/naming_service/helper.cpp
        nelns/naming_service/variables.cpp
)
add_library(nelns::ns ALIAS nelns_naming_service)


ADD_EXECUTABLE(naming_service WIN32
        naming_service.cpp
        nelns/naming_service/helper.cpp
        nelns/naming_service/helper.h
)

TARGET_LINK_LIBRARIES(naming_service
        nelmisc
        nelnet
        nelns::ns
)

NL_DEFAULT_PROPS(naming_service "NeLNS, Services: Naming Service")
NL_ADD_RUNTIME_FLAGS(naming_service)

INSTALL(TARGETS naming_service RUNTIME DESTINATION sbin COMPONENT ns)
INSTALL(FILES naming_service.cfg common.cfg DESTINATION ${NL_ETC_PREFIX}/nelns COMPONENT ns)
