INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${QT_INCLUDES}
			${LIBXML2_INCLUDE_DIR}
			${QtPropertyBrowser_INCLUDE_DIR}
			)

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUGIN_WORLD_EDITOR_HDR	world_editor_plugin.h
					world_editor_window.h
					world_editor_scene.h
					world_editor_scene_item.h
					primitives_model.h
					primitives_view.h
					project_settings_dialog.h
					property_editor_widget.h
					world_editor_settings_page.h
					const_string_array_property.h
					const_string_array_editor.h
)

SET(OVQT_PLUGIN_WORLD_EDITOR_UIS	world_editor_window.ui
					project_settings_dialog.ui
					property_editor_widget.ui
					world_editor_settings_page.ui
					const_string_array_editor.ui
)

SET(OVQT_PLUGIN_WORLD_EDITOR_RCS world_editor.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT5_ADD_RESOURCES(OVQT_PLUGIN_WORLD_EDITOR_RC_SRCS ${OVQT_PLUGIN_WORLD_EDITOR_RCS})
QT5_WRAP_CPP(OVQT_PLUGIN_WORLD_EDITOR_MOC_SRC ${OVQT_PLUGIN_WORLD_EDITOR_HDR})
QT5_WRAP_UI(OVQT_PLUGIN_WORLD_EDITOR_UI_HDRS ${OVQT_PLUGIN_WORLD_EDITOR_UIS})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUGIN_WORLD_EDITOR_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUGIN_WORLD_EDITOR_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocQrcSrc FILES ${OVQT_PLUGIN_WORLD_EDITOR_MOC_SRC} OVQT_PLUGIN_WORLD_EDITOR_RC_SRCS)
SOURCE_GROUP("World Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_world_editor MODULE ${SRC}
						${OVQT_PLUGIN_WORLD_EDITOR_MOC_SRC} 
						${OVQT_EXT_SYS_SRC}
						${OVQT_PLUGIN_WORLD_EDITOR_UI_HDRS}
						${OVQT_PLUGIN_WORLD_EDITOR_RC_SRCS})

TARGET_LINK_LIBRARIES(	studio_plugin_world_editor 
						studio_plugin_core 
						studio_plugin_landscape_editor 
						nelmisc 
						nel3d
						${QtPropertyBrowser_LIBRARY}
						QtPropertyBrowser::Text
						${QT_LIBRARIES}
						${QT_QTOPENGL_LIBRARY}
						${LIBXML2_LIBRARIES}
)

NL_DEFAULT_PROPS(studio_plugin_world_editor "Tools: Studio Plugin: World Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_world_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_world_editor)

ADD_DEFINITIONS(-DWORLD_EDITOR_LIBRARY -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

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

