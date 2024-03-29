INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_PLUG_SHEET_BUILDER_HDR sheetbuilderconfgdialog.h sheetbuilderdialog.h ovqt_sheet_builder.h)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT5_WRAP_CPP(OVQT_PLUG_SHEET_BUILDER_MOC_SRC ${OVQT_PLUG_SHEET_BUILDER_HDR})

SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_PLUG_SHEET_BUILDER_MOC_SRC})
SOURCE_GROUP("Sheet builder Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_sheet_builder MODULE ${SRC} ${OVQT_PLUG_SHEET_BUILDER_MOC_SRC} ${OVQT_EXT_SYS_SRC})

TARGET_LINK_LIBRARIES(studio_plugin_sheet_builder studio_plugin_core nelmisc ${QT_LIBRARIES})

NL_DEFAULT_PROPS(studio_plugin_sheet_builder "Tools: Studio Plugin: Sheet builder")
NL_ADD_RUNTIME_FLAGS(studio_plugin_sheet_builder)
NL_ADD_LIB_SUFFIX(studio_plugin_sheet_builder)

ADD_DEFINITIONS(-DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})


IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_sheet_builder LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_sheet_builder LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_sheet_builder LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_sheet_builder LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_sheet_builder.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

