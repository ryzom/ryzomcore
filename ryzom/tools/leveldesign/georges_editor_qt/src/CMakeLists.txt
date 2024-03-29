INCLUDE_DIRECTORIES(
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		${NEL_INCLUDE_DIR}
		../../../../../nel/tools/3d/object_viewer_widget/src)

FILE(GLOB GEORGES_EDITOR_SRC *.cpp *.h)

SET(GEORGES_EDITOR_HDR
	georges_dirtree_dialog.h
	georges_treeview_dialog.h
	main_window.h
	objectviewer_dialog.h
	settings_dialog.h
	progress_dialog.h
	new_dialog.h
	completer_line_edit.h
	georges_splash.h)

SET(GEORGES_EDITOR_UIS
	settings_form.ui
	objectviewer_form.ui
	log_form.ui
	georges_treeview_form.ui
	georges_dirtree_form.ui
	new_form.ui
	splash.ui)

SET(GEORGES_EDITOR_RCS georges_editor_qt.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT5_ADD_RESOURCES( GEORGES_EDITOR_RC_SRCS ${GEORGES_EDITOR_RCS} )
QT5_WRAP_CPP( GEORGES_EDITOR_MOC_SRCS ${GEORGES_EDITOR_HDR} )
QT5_WRAP_UI( GEORGES_EDITOR_UI_HDRS ${GEORGES_EDITOR_UIS} )

SOURCE_GROUP(QtResources FILES ${GEORGES_EDITOR_UIS} ${GEORGES_EDITOR_RCS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${GEORGES_EDITOR_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${GEORGES_EDITOR_MOC_SRCS})

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

ADD_EXECUTABLE(georges_editor_qt WIN32 MACOSX_BUNDLE ${GEORGES_EDITOR_SRC} ${GEORGES_EDITOR_MOC_SRCS} ${GEORGES_EDITOR_RC_SRCS} ${GEORGES_EDITOR_UI_HDRS})

TARGET_LINK_LIBRARIES(georges_editor_qt
		nelmisc
		nel3d
		nelgeorges
		${QT_LIBRARIES}
		${QT_QTOPENGL_LIBRARY}
		${QT_QTMAIN_LIBRARY})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS ${QT_DEFINITIONS})
NL_DEFAULT_PROPS(georges_editor_qt "Ryzom, Tools, World: Georges Editor Qt")
NL_ADD_RUNTIME_FLAGS(georges_editor_qt)

IF(WITH_PCH)
  target_precompile_headers(georges_editor_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()

INSTALL(TARGETS georges_editor_qt RUNTIME DESTINATION ${RYZOM_BIN_PREFIX} COMPONENT runtime BUNDLE DESTINATION /Applications)
