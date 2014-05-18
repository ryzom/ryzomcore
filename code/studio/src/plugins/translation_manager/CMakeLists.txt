INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
						${CMAKE_CURRENT_SOURCE_DIR}
						${LIBXML2_INCLUDE_DIR}
						${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_TRANSLATION_MANAGER_HDR 	translation_manager_plugin.h
										translation_manager_main_window.h
										translation_manager_settings_page.h
										translation_manager_editor.h
										source_selection.h
										ftp_selection.h
										editor_worksheet.h
										editor_phrase.h
)

SET(OVQT_PLUG_TRANSLATION_MANAGER_UIS	translation_manager_settings_page.ui
										translation_manager_main_window.ui
										source_selection.ui
										ftp_selection.ui)

SET(OVQT_PLUG_TRANSLATION_MANAGER_RCS	ftp_selection.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTNETWORK TRUE)

QT4_WRAP_CPP(OVQT_PLUG_TRANSLATION_MANAGER_MOC_SRC ${OVQT_PLUG_TRANSLATION_MANAGER_HDR})
QT4_WRAP_UI(OVQT_PLUG_TRANSLATION_MANAGER_UI_HDRS ${OVQT_PLUG_TRANSLATION_MANAGER_UIS})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_TRANSLATION_MANAGER_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUG_TRANSLATION_MANAGER_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_PLUG_TRANSLATION_MANAGER_MOC_SRC})
SOURCE_GROUP("Translation Manager Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_translation_manager MODULE ${SRC} ${OVQT_PLUG_TRANSLATION_MANAGER_MOC_SRC} ${OVQT_EXT_SYS_SRC} ${OVQT_PLUG_TRANSLATION_MANAGER_UI_HDRS})

TARGET_LINK_LIBRARIES(studio_plugin_translation_manager studio_plugin_core nelmisc nelligo nelgeorges ${QT_LIBRARIES} ${QT_QTOPENGL_LIBRARY} ${QT_QTNETWORK_LIBRARY} )

NL_DEFAULT_PROPS(studio_plugin_translation_manager "Tools: Studio Plugin: Translation Manager")
NL_ADD_RUNTIME_FLAGS(studio_plugin_translation_manager)
NL_ADD_LIB_SUFFIX(studio_plugin_translation_manager)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_translation_manager LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_translation_manager LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF(WITH_INSTALL_LIBRARIES)
ELSE(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_translation_manager LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_translation_manager LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF(WITH_INSTALL_LIBRARIES)
ENDIF(WIN32)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ovqt_plugin_translation_manager.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

