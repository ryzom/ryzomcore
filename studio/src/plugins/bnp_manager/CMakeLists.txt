INCLUDE_DIRECTORIES(  ${CMAKE_CURRENT_BINARY_DIR}
                      ${CMAKE_CURRENT_SOURCE_DIR}
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
								bnp_manager_window.ui
								)

SET(OVQT_PLUGIN_BNP_MANAGER_RCS bnp_manager.qrc)

SET(QT_USE_QTGUI TRUE)

SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_BNP_MANAGER_UIS} ${OVQT_PLUGIN_BNP_MANAGER_RCS})
SOURCE_GROUP("BNP Manager Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_bnp_manager MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_bnp_manager
		PRIVATE
        bnp_dirtree_dialog.cpp
        bnp_file.cpp
        bnp_filelist_dialog.cpp
        bnp_filesystem_model.cpp
        bnp_manager_plugin.cpp
        bnp_manager_window.cpp
        bnp_proxy_model.cpp
		bnp_manager.qrc
		bnp_dirtree_form.ui
		bnp_filelist_dialog.ui
		bnp_manager_window.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_bnp_manager
		PUBLIC
		Studio::plugin::core
		PRIVATE
		NeL::misc
		NeL::georges
		Qt::Core
		Qt::Gui
		Qt::Widgets
)

NL_DEFAULT_PROPS(studio_plugin_bnp_manager "Tools: Studio Plugin: BNP Manager")
NL_ADD_RUNTIME_FLAGS(studio_plugin_bnp_manager)
NL_ADD_LIB_SUFFIX(studio_plugin_bnp_manager)

TARGET_COMPILE_DEFINITIONS(studio_plugin_bnp_manager PRIVATE QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})

INSTALL(TARGETS studio_plugin_bnp_manager LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} ARCHIVE DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_bnp_manager.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

