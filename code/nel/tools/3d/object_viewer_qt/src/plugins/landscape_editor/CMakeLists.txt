INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${LIBXML2_INCLUDE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUGIN_LANDSCAPE_EDITOR_HDR	landscape_editor_plugin.h
					landscape_editor_window.h
					landscape_scene_base.h
					landscape_scene.h
					list_zones_model.h
					list_zones_widget.h
					landscape_view.h
					project_settings_dialog.h
					snapshot_dialog.h
)

SET(OVQT_PLUGIN_LANDSCAPE_EDITOR_UIS	landscape_editor_window.ui
					list_zones_widget.ui
					project_settings_dialog.ui
					shapshot_dialog.ui
)

SET(OVQT_PLUGIN_LANDSCAPE_EDITOR_RCS landscape_editor.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_ADD_RESOURCES(OVQT_PLUGIN_LANDSCAPE_EDITOR_RC_SRCS ${OVQT_PLUGIN_LANDSCAPE_EDITOR_RCS})
QT4_WRAP_CPP(OVQT_PLUGIN_LANDSCAPE_EDITOR_MOC_SRC ${OVQT_PLUGIN_LANDSCAPE_EDITOR_HDR})
QT4_WRAP_UI(OVQT_PLUGIN_LANDSCAPE_EDITOR_UI_HDRS ${OVQT_PLUGIN_LANDSCAPE_EDITOR_UIS})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUGIN_LANDSCAPE_EDITOR_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUGIN_LANDSCAPE_EDITOR_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocQrcSrc FILES ${OVQT_PLUGIN_LANDSCAPE_EDITOR_MOC_SRC} OVQT_PLUGIN_LANDSCAPE_EDITOR_RC_SRCS)
SOURCE_GROUP("Landscape Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(ovqt_plugin_landscape_editor MODULE ${SRC}
						${OVQT_PLUGIN_LANDSCAPE_EDITOR_MOC_SRC} 
						${OVQT_EXT_SYS_SRC}
						${OVQT_PLUGIN_LANDSCAPE_EDITOR_UI_HDRS}
						${OVQT_PLUGIN_LANDSCAPE_EDITOR_RC_SRCS})

TARGET_LINK_LIBRARIES(ovqt_plugin_landscape_editor ovqt_plugin_core nelmisc nel3d nelgeorges nelligo ${QT_LIBRARIES} ${QT_QTOPENGL_LIBRARY})

NL_DEFAULT_PROPS(ovqt_plugin_landscape_editor "NeL, Tools, 3D: Object Viewer Qt Plugin: Landscape Editor")
NL_ADD_RUNTIME_FLAGS(ovqt_plugin_landscape_editor)
NL_ADD_LIB_SUFFIX(ovqt_plugin_landscape_editor)

ADD_DEFINITIONS(-DLANDSCAPE_EDITOR_LIBRARY ${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

INSTALL(TARGETS ovqt_plugin_landscape_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} ARCHIVE DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
#INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ovqt_plugin_landscape_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)
