INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_LOG_UIS	log_form.ui
						log_settings_page.ui)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_LOG_UIS})
SOURCE_GROUP("Log Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_log MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_log
        PRIVATE
        log_plugin.cpp
        log_settings_page.cpp
        qt_displayer.cpp
        log_form.ui
        log_settings_page.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_log
        PRIVATE
        Studio::plugin::core
        NeL::misc
        Qt::Core
        Qt::Widgets
)

NL_DEFAULT_PROPS(studio_plugin_log "Tools: Studio Plugin: Log")
NL_ADD_RUNTIME_FLAGS(studio_plugin_log)
NL_ADD_LIB_SUFFIX(studio_plugin_log)

TARGET_COMPILE_DEFINITIONS(studio_plugin_log PRIVATE QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})


IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_log LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_log LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_log LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_log LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_log.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

