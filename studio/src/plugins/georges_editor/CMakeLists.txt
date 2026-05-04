FIND_PACKAGE(QtPropertyBrowser REQUIRED)

INCLUDE_DIRECTORIES(  ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}
                      ${QT_INCLUDES}
                      ${QtPropertyBrowser_INCLUDE_DIR})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
                      ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
                      ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_GEORGES_EDITOR_HDR  georges_editor_plugin.h 
                                  georges_editor_form.h
                                  georges_dirtree_dialog.h
                                  georges_filesystem_model.h
                                  georges_treeview_dialog.h
                                  expandable_headerview.h
								  browser_ctrl.h
								  browser_ctrl_pvt.h
								  dfn_browser_ctrl.h
								  georges_dfn_dialog.h
								  filepath_property_manager.h
								  typ_browser_ctrl.h
								  georges_typ_dialog.h
								  georges_dock_widget.h
								  )

SET(OVQT_PLUG_GEORGES_EDITOR_UIS  georges_editor_form.ui
                                  georges_dirtree_form.ui
                                  georges_treeview_form.ui
								  georges_dfn_dialog.ui
								  georges_typ_dialog.ui
								  )

SET(OVQT_PLUGIN_GEORGES_EDITOR_RCS georges_editor.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)


SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_GEORGES_EDITOR_UIS} ${OVQT_PLUGIN_GEORGES_EDITOR_RCS})
SOURCE_GROUP("Georges Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_georges_editor MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_georges_editor
        PRIVATE
        actions.cpp
        browser_ctrl.cpp
        browser_ctrl_pvt.cpp
        dfn_browser_ctrl.cpp
        expandable_headerview.cpp
        filepath_property_manager.cpp
        formdelegate.cpp
        formitem.cpp
        georges.cpp
        georges_dfn_dialog.cpp
        georges_dirtree_dialog.cpp
        georges_dock_widget.cpp
        georges_editor_form.cpp
        georges_editor_plugin.cpp
        georges_filesystem_model.cpp
        georges_treeview_dialog.cpp
        georges_typ_dialog.cpp
        georgesform_model.cpp
        georgesform_proxy_model.cpp
        stdpch.cpp
        typ_browser_ctrl.cpp
        georges_dfn_dialog.ui
        georges_dirtree_form.ui
        georges_editor_form.ui
        georges_treeview_form.ui
        georges_typ_dialog.ui
        georges_editor.qrc
)

TARGET_LINK_LIBRARIES(studio_plugin_georges_editor
        PRIVATE
        Studio::plugin::core
        NeL::misc
        NeL::georges
        qtpropertybrowser
        ${QtPropertyBrowser_LIBRARY}
		Qt::Core
		Qt::Widgets
        LibXml2::LibXml2
)

NL_DEFAULT_PROPS(studio_plugin_georges_editor "Tools: Studio Plugin: Georges Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_georges_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_georges_editor)

TARGET_COMPILE_DEFINITIONS(studio_plugin_georges_editor PRIVATE QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})


IF(WITH_PCH)
  target_precompile_headers(studio_plugin_georges_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_georges_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

