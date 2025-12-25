FILE(GLOB CRASHREPORT_SRC *.cpp *.rc)
FILE(GLOB CRASHREPORT_HDR *.h)

SET(CRASHREPORT_QRC resources.qrc)
SET(CRASHREPORT_UI crash_report_widget.ui)

SOURCE_GROUP(QtResources FILES ${CRASHREPORT_UI} ${CRASHREPORT_QRC})
SOURCE_GROUP("source files" FILES ${CRASHREPORT_SRC})
SOURCE_GROUP("header files" FILES ${CRASHREPORT_HDR})

ADD_EXECUTABLE(crash_report WIN32)
target_sources(crash_report
        PRIVATE
        crash_report.cpp
        crash_report.rc
        crash_report_socket.cpp
        crash_report_widget.cpp
        crash_report_widget.ui
        resources.qrc
)

TARGET_INCLUDE_DIRECTORIES(crash_report PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(crash_report
        PRIVATE
        Qt::Core
        Qt::Network
        Qt::Widgets
)

NL_DEFAULT_PROPS(crash_report "NeL, Tools, Misc: Crash Report")
NL_ADD_RUNTIME_FLAGS(crash_report)

INSTALL(TARGETS crash_report RUNTIME DESTINATION ${NL_BIN_PREFIX})
