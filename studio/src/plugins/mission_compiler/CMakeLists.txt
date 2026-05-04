INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
						${CMAKE_CURRENT_SOURCE_DIR}
						${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(QT_USE_QTGUI TRUE)

SOURCE_GROUP("Mission Compiler Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

# Game Share Library
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/common/src)

ADD_LIBRARY(studio_plugin_mission_compiler MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_mission_compiler
        PRIVATE
        mission_compiler_main_window.cpp
        mission_compiler_plugin.cpp
        mission_compiler_settings_page.cpp
        server_entry_dialog.cpp
        validation_file.cpp
        mission_compiler.qrc
        mission_compiler_main_window.ui
        mission_compiler_settings_page.ui
        server_entry_dialog.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_mission_compiler
        PRIVATE
        Studio::plugin::core
        NeL::misc
        NeL::ligo
        Ryzom::mission_compiler_lib
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

NL_DEFAULT_PROPS(studio_plugin_mission_compiler "Tools: Studio Plugin: Mission Compiler")
NL_ADD_RUNTIME_FLAGS(studio_plugin_mission_compiler)
NL_ADD_LIB_SUFFIX(studio_plugin_mission_compiler)

TARGET_COMPILE_DEFINITIONS(studio_plugin_mission_compiler PRIVATE QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})



IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_mission_compiler LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_mission_compiler LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_mission_compiler LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_mission_compiler LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_mission_compiler.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

