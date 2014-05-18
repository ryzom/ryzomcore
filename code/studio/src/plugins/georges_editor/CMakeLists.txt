INCLUDE_DIRECTORIES(  ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}
                      ${LIBXML2_INCLUDE_DIR}
                      ${QT_INCLUDES}
					  ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qtpropertybrowser)

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
                      ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
                      ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h
					  ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_GEORGES_EDITOR_HDR  georges_editor_plugin.h 
                                  georges_editor_form.h
                                  georges_dirtree_dialog.h
                                  georges_filesystem_model.h
                                  georges_treeview_dialog.h
                                  expandable_headerview.h)

SET(OVQT_PLUG_GEORGES_EDITOR_UIS  georges_editor_form.ui
                                  georges_dirtree_form.ui
                                  georges_treeview_form.ui)

SET(OVQT_PLUGIN_GEORGES_EDITOR_RCS georges_editor.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_ADD_RESOURCES(OVQT_PLUGIN_GEORGES_EDITOR_RC_SRCS ${OVQT_PLUGIN_GEORGES_EDITOR_RCS})
QT4_WRAP_CPP(OVQT_PLUG_GEORGES_EDITOR_MOC_SRC ${OVQT_PLUG_GEORGES_EDITOR_HDR})
QT4_WRAP_UI( OVQT_PLUG_GEORGES_EDITOR_UI_HDRS ${OVQT_PLUG_GEORGES_EDITOR_UIS} )

SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_GEORGES_EDITOR_UIS} ${OVQT_PLUGIN_GEORGES_EDITOR_RCS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUG_GEORGES_EDITOR_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_PLUG_GEORGES_EDITOR_MOC_SRC})
SOURCE_GROUP("Georges Editor Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_georges_editor MODULE ${SRC} ${OVQT_PLUG_GEORGES_EDITOR_MOC_SRC} ${OVQT_EXT_SYS_SRC} ${OVQT_PLUG_GEORGES_EDITOR_UI_HDRS} ${OVQT_PLUGIN_GEORGES_EDITOR_RC_SRCS})

TARGET_LINK_LIBRARIES(studio_plugin_georges_editor studio_plugin_core nelmisc nelgeorges qt_property_browser ${QT_LIBRARIES})

NL_DEFAULT_PROPS(studio_plugin_georges_editor "Tools: Studio Plugin: Georges Editor")
NL_ADD_RUNTIME_FLAGS(studio_plugin_georges_editor)
NL_ADD_LIB_SUFFIX(studio_plugin_georges_editor)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})


IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(studio_plugin_georges_editor ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF(WITH_INSTALL_LIBRARIES)
ELSE(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_georges_editor LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF(WITH_INSTALL_LIBRARIES)
ENDIF(WIN32)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ovqt_plugin_georges_editor.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

