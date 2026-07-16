add_executable(nel_launcher_qt WIN32)

target_sources(nel_launcher_qt
		PRIVATE
		connection.cpp
		main.cpp
		nel_launcher_dlg.cpp
		nel_launcher_dlg.ui
)
target_include_directories(nel_launcher_qt
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(nel_launcher_qt
        PRIVATE
		Qt::Widgets
        NeL::misc
        NeL::net
)

NL_DEFAULT_PROPS(nel_launcher_qt "NelNS, Launcher: NeL Launcher Qt")
NL_ADD_RUNTIME_FLAGS(nel_launcher_qt)

INSTALL(TARGETS nel_launcher_qt RUNTIME DESTINATION bin COMPONENT launcher)
IF (WIN32)
    INSTALL(FILES nel_launcher.cfg DESTINATION bin COMPONENT launcher)
ELSE ()
    INSTALL(FILES nel_launcher.cfg DESTINATION ${NL_ETC_PREFIX}/nelns COMPONENT launcher)
ENDIF ()
