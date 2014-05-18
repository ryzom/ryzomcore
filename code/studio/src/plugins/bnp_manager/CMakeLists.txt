INCLUDE_DIRECTORIES(  ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}
                      ${LIBXML2_INCLUDE_DIR}
                      ${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
                      ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
                      ${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_BNP_MANAGER_HDR  bnp_manager_plugin.h 
                                  bnp_manager_window.h
								  bnp_dirtree_dialog.h
								  bnp_filesystem_model.h
								  bnp_file.h
								  bnp_filelist_dialog.h
								  bnp_proxy_model.h
                                  )
SET(OVQT_PLUG_BNP_MANAGER_UIS	bnp_dirtree_form.ui
								bnp_filelist_dialog.ui
								)

SET(OVQT_PLUGIN_BNP_MANAGER_RCS bnp_manager.qrc)

SET(QT_USE_QTGUI TRUE)

QT4_ADD_RESOURCES(OVQT_PLUGIN_BNP_MANAGER_RC_SRCS ${OVQT_PLUGIN_BNP_MANAGER_RCS})
QT4_WRAP_CPP(OVQT_PLUG_BNP_MANAGER_MOC_SRC ${OVQT_PLUG_BNP_MANAGER_HDR})
QT4_WRAP_UI(OVQT_PLUG_BNP_MANAGER_UI_HDRS ${OVQT_PLUG_BNP_MANAGER_UIS})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_BNP_MANAGER_UIS} ${OVQT_PLUGIN_BNP_MANAGER_RCS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_PLUG_BNP_MANAGER_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_PLUG_BNP_MANAGER_MOC_SRC})
SOURCE_GROUP("BNP Manager Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_bnp_manager MODULE ${SRC} ${OVQT_PLUG_BNP_MANAGER_MOC_SRC} ${OVQT_EXT_SYS_SRC} ${OVQT_PLUGIN_BNP_MANAGER_RC_SRCS} ${OVQT_PLUG_BNP_MANAGER_UI_HDRS})

TARGET_LINK_LIBRARIES(studio_plugin_bnp_manager studio_plugin_core nelmisc nelgeorges ${QT_LIBRARIES})

NL_DEFAULT_PROPS(studio_plugin_bnp_manager "Tools: Studio Plugin: BNP Manager")
NL_ADD_RUNTIME_FLAGS(studio_plugin_bnp_manager)
NL_ADD_LIB_SUFFIX(studio_plugin_bnp_manager)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

INSTALL(TARGETS studio_plugin_bnp_manager LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} ARCHIVE DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_bnp_manager.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

