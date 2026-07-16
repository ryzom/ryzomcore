SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

if(APPLE)
  SET(MACOSX_BUNDLE_INFO_STRING "Georges Editor Qt")
  SET(MACOSX_BUNDLE_ICON_FILE "georges_logo.icns")
  SET(MACOSX_BUNDLE_GUI_IDENTIFIER "")
  SET(MACOSX_BUNDLE_LONG_VERSION_STRING ${NL_VERSION})
  SET(MACOSX_BUNDLE_BUNDLE_NAME "Georges Editor Qt")
  SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "0.9")
  SET(MACOSX_BUNDLE_BUNDLE_VERSION "0.9")
  SET(MACOSX_BUNDLE_COPYRIGHT ${AUTHOR})
ENDIF()

ADD_EXECUTABLE(georges_editor_qt WIN32 MACOSX_BUNDLE)

target_sources(georges_editor_qt
        PRIVATE
        completer_line_edit.cpp
        configuration.cpp
        entity.cpp
        filesystem_model.cpp
        formdelegate.cpp
        formitem.cpp
        georges.cpp
        georges_dirtree_dialog.cpp
        georges_splash.cpp
        georges_treeview_dialog.cpp
        georgesform_model.cpp
        georgesform_proxy_model.cpp
        log_dialog.cpp
        main.cpp
        main_window.cpp
        modules.cpp
        new_dialog.cpp
        objectviewer_dialog.cpp
        progress_dialog.cpp
        qt_displayer.cpp
        settings_dialog.cpp
        stdpch.cpp
        georges_editor_qt.qrc
        georges_dirtree_form.ui
        georges_treeview_form.ui
        log_form.ui
        new_form.ui
        objectviewer_form.ui
        settings_form.ui
        splash.ui
)

TARGET_INCLUDE_DIRECTORIES(georges_editor_qt
		PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		../../../../../nel/tools/3d/object_viewer_widget/src
)


TARGET_LINK_LIBRARIES(georges_editor_qt
		PRIVATE
		NeL::misc
		NeL::3d
		NeL::georges
		Qt::Core
		Qt::OpenGL
		Qt::Widgets
)

TARGET_COMPILE_DEFINITIONS(georges_editor_qt PRIVATE QT_NO_KEYWORDS ${QT_DEFINITIONS})
NL_DEFAULT_PROPS(georges_editor_qt "Ryzom, Tools, World: Georges Editor Qt")
NL_ADD_RUNTIME_FLAGS(georges_editor_qt)

IF(WITH_PCH)
  target_precompile_headers(georges_editor_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS georges_editor_qt RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT runtime BUNDLE DESTINATION /Applications)
