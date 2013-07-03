INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_INCLUDES}
	)

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)
			
SET(OVQT_PLUGIN_MATERIAL_EDITOR_HDR
	material_editor_context.h
	material_editor_plugin.h
	material_editor_window.h
	material_editor_core_listener.h
	material_properties.h
	material_property_editor.h
	material_widget.h
	material_splitter.h
	render_passes.h
	shader_editor.h
	shader_widget.h
	prop_browser_ctrl.h
	)
	
SET(OVQT_PLUGIN_MATERIAL_EDITOR_UIS
	material_editor_window.ui
	material_properties.ui
	material_property_editor.ui
	material_widget.ui
	render_passes.ui
	shader_editor.ui
	shader_widget.ui
	)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_ADD_RESOURCES(OVQT_PLUGIN_MATERIAL_EDITOR_RC_SRCS ${OVQT_PLUGIN_MATERIAL_EDITOR_RCS})
QT4_WRAP_CPP(OVQT_PLUGIN_MATERIAL_EDITOR_MOC_SRC ${OVQT_PLUGIN_MATERIAL_EDITOR_HDR})
QT4_WRAP_UI(OVQT_PLUGIN_MATERIAL_EDITOR_UI_HDRS ${OVQT_PLUGIN_MATERIAL_EDITOR_UIS})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUGIN_MATERIAL_EDITOR_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUGIN_MATERIAL_EDITOR_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocQrcSrc FILES ${OVQT_PLUGIN_MATERIAL_EDITOR_MOC_SRC} OVQT_PLUGIN_MATERIAL_EDITOR_RC_SRCS)
SOURCE_GROUP("Material Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(ovqt_plugin_material_editor MODULE ${SRC}
            ${OVQT_PLUGIN_MATERIAL_EDITOR_MOC_SRC}
            ${OVQT_EXT_SYS_SRC}
            ${OVQT_PLUGIN_MATERIAL_EDITOR_UI_HDRS}
            ${OVQT_PLUGIN_MATERIAL_EDITOR_RC_SRCS}
            )

TARGET_LINK_LIBRARIES(
						ovqt_plugin_material_editor
						ovqt_plugin_core
						qt_property_browser
						nelmisc
						nel3d
						${QT_LIBRARIES}
						${QT_QTOPENGL_LIBRARY}
						)

NL_DEFAULT_PROPS(ovqt_plugin_material_editor "NeL, Tools, 3D: Object Viewer Qt Plugin: Material Editor")
NL_ADD_RUNTIME_FLAGS(ovqt_plugin_material_editor)
NL_ADD_LIB_SUFFIX(ovqt_plugin_material_editor)

ADD_DEFINITIONS(-DMATERIAL_EDITOR_LIBRARY -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

INSTALL(TARGETS ovqt_plugin_material_editor LIBRARY DESTINATION lib RUNTIME DESTINATION bin ARCHIVE DESTINATION lib COMPONENT tools3d)
