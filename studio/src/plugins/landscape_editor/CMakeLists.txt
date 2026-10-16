INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUGIN_LANDSCAPE_EDITOR_UIS	landscape_editor_window.ui
					list_zones_widget.ui
					project_settings_dialog.ui
					shapshot_dialog.ui
)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

SOURCE_GROUP(QtResources FILES ${OVQT_PLUGIN_LANDSCAPE_EDITOR_UIS})
SOURCE_GROUP("Landscape Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_landscape_editor SHARED ${OVQT_EXT_SYS_SRC})

add_library(Studio::plugin::LandscapeEditor ALIAS studio_plugin_landscape_editor)

target_sources(studio_plugin_landscape_editor
        PRIVATE
        builder_zone.cpp
        builder_zone_base.cpp
        builder_zone_region.cpp
        landscape_actions.cpp
        landscape_editor_plugin.cpp
        landscape_editor_window.cpp
        landscape_scene.cpp
        landscape_scene_base.cpp
        landscape_view.cpp
        list_zones_model.cpp
        list_zones_widget.cpp
        pixmap_database.cpp
        project_settings_dialog.cpp
        snapshot_dialog.cpp
        zone_region_editor.cpp
        landscape_editor.qrc
        landscape_editor_window.ui
        list_zones_widget.ui
        project_settings_dialog.ui
        shapshot_dialog.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_landscape_editor
        PUBLIC
        NeL::ligo
        PRIVATE
		Studio::plugin::core
		NeL::misc
		NeL::3d
		NeL::georges
		Qt::Core
		Qt::OpenGL
		Qt::Widgets
)

NL_DEFAULT_PROPS(studio_plugin_landscape_editor "Tools: Studio Plugin: Landscape Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_landscape_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_landscape_editor)

TARGET_COMPILE_DEFINITIONS(studio_plugin_landscape_editor PRIVATE LANDSCAPE_EDITOR_LIBRARY QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})



IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_landscape_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_landscape_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_landscape_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_landscape_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_landscape_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)
