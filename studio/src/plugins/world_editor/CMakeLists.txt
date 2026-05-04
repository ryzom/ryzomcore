INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${QT_INCLUDES}
			${QtPropertyBrowser_INCLUDE_DIR}
			)

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

SOURCE_GROUP("World Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_world_editor MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_world_editor
        PRIVATE
        const_string_array_editor.cpp
        const_string_array_property.cpp
        primitive_item.cpp
        primitives_model.cpp
        primitives_view.cpp
        project_settings_dialog.cpp
        property_editor_widget.cpp
        world_editor_actions.cpp
        world_editor_misc.cpp
        world_editor_plugin.cpp
        world_editor_scene.cpp
        world_editor_scene_item.cpp
        world_editor_settings_page.cpp
        world_editor_window.cpp
        world_editor.qrc
        const_string_array_editor.ui
        project_settings_dialog.ui
        property_editor_widget.ui
        world_editor_settings_page.ui
        world_editor_window.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_world_editor
        PRIVATE
        Studio::plugin::core
        Studio::plugin::LandscapeEditor
		NeL::misc
		NeL::3d
		${QtPropertyBrowser_LIBRARY}
		QtPropertyBrowser::Text
		Qt::Core
		Qt::Gui
		Qt::OpenGL
		Qt::Widgets
		LibXml2::LibXml2
)

NL_DEFAULT_PROPS(studio_plugin_world_editor "Tools: Studio Plugin: World Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_world_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_world_editor)

TARGET_COMPILE_DEFINITIONS(studio_plugin_world_editor PRIVATE WORLD_EDITOR_LIBRARY QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_world_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_world_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_world_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_world_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_world_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

