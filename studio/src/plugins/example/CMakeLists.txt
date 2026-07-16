INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
						${CMAKE_CURRENT_SOURCE_DIR}
						${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
						${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_EXAMPLE_HDR 	example_plugin.h
				qnel_widget.h
				simple_viewer.h
				example_settings_page.h)

SET(OVQT_PLUG_EXAMPLE_UIS	example_settings_page.ui)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT5_WRAP_CPP(OVQT_PLUG_EXAMPLE_MOC_SRC ${OVQT_PLUG_EXAMPLE_HDR})

SOURCE_GROUP(QtResources FILES ${OVQT_PLUG_EXAMPLE_UIS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_PLUG_EXAMPLE_MOC_SRC})
SOURCE_GROUP("Example Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_example MODULE ${OVQT_PLUG_EXAMPLE_MOC_SRC} ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_example
        PRIVATE
        example_plugin.cpp
        example_settings_page.cpp
        qnel_widget.cpp
        simple_viewer.cpp
        example_settings_page.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_example
        PRIVATE
        Studio::plugin::core
        NeL::misc
        NeL::3d
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::Widgets
)

NL_DEFAULT_PROPS(studio_plugin_example "Tools: Studio Plugin: Example")
NL_ADD_RUNTIME_FLAGS(studio_plugin_example)
NL_ADD_LIB_SUFFIX(studio_plugin_example)

TARGET_COMPILE_DEFINITIONS(studio_plugin_example PRIVATE QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})

IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_example LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_example LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_example LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_example LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_example.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

