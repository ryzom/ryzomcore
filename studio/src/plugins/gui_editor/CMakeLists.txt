INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${LUA_INCLUDE_DIR})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/gui_editor_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/gui_editor_config.h)

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUGIN_GUI_EDITOR_HDR
	gui_editor_plugin.h
	gui_editor_window.h
	gui_editor_context.h
	gui_editor_core_listener.h
	widget_properties.h
	widget_hierarchy.h
	link_list.h
	link_editor.h
	proc_list.h
	proc_editor.h
	action_editor.h
	property_browser_ctrl.h
	project_window.h
	nelgui_ctrl.h
	new_property_widget.h
	new_widget_widget.h
	add_widget_widget.h
	editor_selection_watcher.h
	editor_message_processor.h
	action_list.h
	texture_chooser.h
	action_property_manager.h
	texture_property_manager.h
	expression_editor.h
	expr_link_dlg.h
	new_gui_dlg.h
	)
	
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

QT5_ADD_RESOURCES(OVQT_PLUGIN_GUI_EDITOR_RC_SRCS ${OVQT_PLUGIN_GUI_EDITOR_RCS})
QT5_WRAP_CPP(OVQT_PLUGIN_GUI_EDITOR_MOC_SRC ${OVQT_PLUGIN_GUI_EDITOR_HDR})
QT5_WRAP_UI(OVQT_PLUGIN_GUI_EDITOR_UI_HDRS ${OVQT_PLUGIN_GUI_EDITOR_UIS})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUGIN_GUI_EDITOR_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUGIN_GUI_EDITOR_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocQrcSrc FILES ${OVQT_PLUGIN_GUI_EDITOR_MOC_SRC} OVQT_PLUGIN_GUI_EDITOR_RC_SRCS)
SOURCE_GROUP("GUI Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_gui_editor MODULE ${SRC}
            ${OVQT_PLUGIN_GUI_EDITOR_MOC_SRC}
            ${OVQT_EXT_SYS_SRC}
            ${OVQT_PLUGIN_GUI_EDITOR_UI_HDRS}
            ${OVQT_PLUGIN_GUI_EDITOR_RC_SRCS}
            )

TARGET_LINK_LIBRARIES(
						studio_plugin_gui_editor
						studio_plugin_core
						nelmisc
						nel3d
						nelgui
						${QT_LIBRARIES}
						${QT_QTOPENGL_LIBRARY}
						${QtPropertyBrowser_LIBRARY}
						)

NL_DEFAULT_PROPS(studio_plugin_gui_editor "Tools: Studio Plugin: GUI Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_gui_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_gui_editor)

ADD_DEFINITIONS(-DGUI_EDITOR_LIBRARY ${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

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
