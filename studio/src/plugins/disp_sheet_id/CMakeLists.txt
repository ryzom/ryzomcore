INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)
SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OVQT_DISP_SHEET_ID_PLUGIN_HDR
			sheet_id_view.h
			disp_sheet_id_plugin.h)

SET(OVQT_DISP_SHEET_ID_PLUGIN_UIS
				sheet_id_view.ui)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT5_WRAP_CPP(OVQT_DISP_SHEET_ID_PLUGIN_MOC_SRC ${OVQT_DISP_SHEET_ID_PLUGIN_HDR})
QT5_WRAP_UI(OVQT_DISP_SHEET_ID_PLUGIN_UI_HDRS ${OVQT_DISP_SHEET_ID_PLUGIN_UIS})

SOURCE_GROUP(QtGeneratedUiHdr FILES ${OVQT_DISP_SHEET_ID_PLUGIN_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OVQT_DISP_SHEET_ID_PLUGIN_MOC_SRC})
SOURCE_GROUP("Display sheet id Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(studio_plugin_disp_sheet_id MODULE ${SRC} ${OVQT_DISP_SHEET_ID_PLUGIN_MOC_SRC} ${OVQT_EXT_SYS_SRC} ${OVQT_DISP_SHEET_ID_PLUGIN_UI_HDRS})

TARGET_LINK_LIBRARIES(studio_plugin_disp_sheet_id studio_plugin_core nelmisc ${QT_LIBRARIES})

IF(WITH_STLPORT)
  TARGET_LINK_LIBRARIES(studio_plugin_disp_sheet_id ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

NL_DEFAULT_PROPS(studio_plugin_disp_sheet_id "Tools: Studio Plugin: Display sheet id")
NL_ADD_RUNTIME_FLAGS(studio_plugin_disp_sheet_id)
NL_ADD_LIB_SUFFIX(studio_plugin_disp_sheet_id)

ADD_DEFINITIONS(-DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})


IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_disp_sheet_id LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_disp_sheet_id LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_disp_sheet_id LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_disp_sheet_id LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_disp_sheet_id.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

