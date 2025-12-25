CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1)
CMAKE_POLICY(SET CMP0015 NEW)

FILE(GLOB SRC htmlcss_test.cpp)
ADD_EXECUTABLE(htmlcss_test ${SRC})
TARGET_INCLUDE_DIRECTORIES(htmlcss_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${LUA_INCLUDE_DIR} )
TARGET_LINK_LIBRARIES(htmlcss_test
        PRIVATE
        nelmisc
        nelgui
        ${LUA_LIBRARIES}
        LibXml2::LibXml2
)
TARGET_LINK_DIRECTORIES(htmlcss_test PRIVATE ${LINK_DIRECTORIES} ${CMAKE_LIBRARY_DIR})

NL_DEFAULT_PROPS(htmlcss_test "Ryzom, Tests: html/css parser tests")
NL_ADD_RUNTIME_FLAGS(htmlcss_test)

INSTALL(TARGETS htmlcss_test   RUNTIME DESTINATION bin COMPONENT tools)
# TODO: test fixtures
