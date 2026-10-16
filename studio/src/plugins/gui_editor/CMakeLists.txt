INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${QT_INCLUDES}
  ${LUA_INCLUDE_DIR})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/gui_editor_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/gui_editor_config.h)

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUGIN_GUI_EDITOR_UIS
	gui_editor_window.ui
	widget_properties.ui
	widget_hierarchy.ui
	link_list.ui
	link_editor.ui
	proc_list.ui
	proc_editor.ui
	action_editor.ui
	project_window.ui
	new_property_widget.ui
	new_widget_widget.ui
	add_widget_widget.ui
	action_list.ui
	texture_chooser.ui
	expression_editor.ui
	expr_link_dlg.ui
	new_gui_dlg.ui
	)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

SOURCE_GROUP(QtResources FILES ${OVQT_PLUGIN_GUI_EDITOR_UIS})
SOURCE_GROUP("GUI Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_gui_editor MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_gui_editor
        PRIVATE
        action_editor.cpp
        action_list.cpp
        action_property_manager.cpp
        add_widget_widget.cpp
        editor_message_processor.cpp
        editor_selection_watcher.cpp
        expr_link_dlg.cpp
        expression_editor.cpp
        expression_link.cpp
        expression_loader.cpp
        expression_node.cpp
        expression_store.cpp
        gui_editor_context.cpp
        gui_editor_core_listener.cpp
        gui_editor_plugin.cpp
        gui_editor_window.cpp
        link_editor.cpp
        link_list.cpp
        nelgui_ctrl.cpp
        new_gui_dlg.cpp
        new_property_widget.cpp
        new_widget_widget.cpp
        proc_editor.cpp
        proc_list.cpp
        project_file_parser.cpp
        project_file_serializer.cpp
        project_window.cpp
        property_browser_ctrl.cpp
        texture_chooser.cpp
        texture_property_manager.cpp
        widget_hierarchy.cpp
        widget_info_serializer.cpp
        widget_properties.cpp
        widget_properties_parser.cpp
        widget_serializer.cpp

        action_editor.ui
        action_list.ui
        add_widget_widget.ui
        expr_link_dlg.ui
        expression_editor.ui
        gui_editor_window.ui
        link_editor.ui
        link_list.ui
        new_gui_dlg.ui
        new_property_widget.ui
        new_widget_widget.ui
        proc_editor.ui
        proc_list.ui
        project_window.ui
        texture_chooser.ui
        widget_hierarchy.ui
        widget_properties.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_gui_editor
        PUBLIC
        Studio::plugin::core
        PRIVATE
		NeL::misc
		NeL::3d
		NeL::gui
		Qt::Core
		Qt::Gui
		Qt::Widgets
        ${QtPropertyBrowser_LIBRARY}
        LibXml2::LibXml2
)

NL_DEFAULT_PROPS(studio_plugin_gui_editor "Tools: Studio Plugin: GUI Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_gui_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_gui_editor)

TARGET_COMPILE_DEFINITIONS(studio_plugin_gui_editor PRIVATE GUI_EDITOR_LIBRARY QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_gui_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_gui_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_gui_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_gui_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_gui_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

FILE(GLOB widgets "${CMAKE_CURRENT_SOURCE_DIR}/widgets/*.xml")
FILE(GLOB expressions "${CMAKE_CURRENT_SOURCE_DIR}/expressions/*.xml")

INSTALL(FILES ${widgets} DESTINATION "${OVQT_DATA_DIR}/widgets" COMPONENT tools3d)
INSTALL(FILES ${expressions} DESTINATION "${OVQT_DATA_DIR}/expressions" COMPONENT tools3d)

INSTALL(DIRECTORY fonts/ DESTINATION ${OVQT_DATA_DIR} COMPONENT data)
