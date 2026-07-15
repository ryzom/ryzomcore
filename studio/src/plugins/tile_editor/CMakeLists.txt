INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

SOURCE_GROUP("Tile Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_tile_editor MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_tile_editor
        PRIVATE
        land_edit_dialog.cpp
        tile_bank.cpp
        tile_editor_main_window.cpp
        tile_editor_plugin.cpp
        tile_item.cpp
        tile_item_delegate.cpp
        tile_model.cpp
        tilebank_loader.cpp
        tilebank_saver.cpp
        tile_editor.qrc
        land_edit_dialog.ui
        tile_editor_main_window.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_tile_editor
        PRIVATE
        Studio::plugin::core
        NeL::misc
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

IF(WITH_STLPORT)
  TARGET_LINK_LIBRARIES(studio_plugin_tile_editor ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

NL_DEFAULT_PROPS(studio_plugin_tile_editor "Tools: Studio Plugin: Tile Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_tile_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_tile_editor)

TARGET_COMPILE_DEFINITIONS(studio_plugin_tile_editor PRIVATE QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_tile_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_tile_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_tile_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_tile_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_tile_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)
