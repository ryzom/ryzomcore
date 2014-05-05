ADD_SUBDIRECTORY(3rdparty)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR} ${QT_INCLUDES})
INCLUDE( ${QT_USE_FILE} )

CONFIGURE_FILE(translations/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc COPYONLY)

FILE(GLOB OBJECT_VIEWER_SRC extension_system/*.h
							extension_system/*.cpp 
							*.h *.cpp)

SET(OBJECT_VIEWER_HDR   extension_system/iplugin_manager.h 
						extension_system/plugin_manager.h)

SET(OBJECT_VIEWER_RCS object_viewer_qt.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

SET(OBJECT_VIEWER_TS    translations/object_viewer_qt_en.ts
						translations/object_viewer_qt_fr.ts
						translations/object_viewer_qt_de.ts
						translations/object_viewer_qt_ru.ts)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

IF(WIN32)
  SET(OBJECT_VIEWER_RC object_viewer_qt.rc)
ENDIF(WIN32)

QT4_ADD_TRANSLATION(OBJECT_VIEWER_QM ${OBJECT_VIEWER_TS})
QT4_ADD_RESOURCES( OBJECT_VIEWER_RC_SRCS ${OBJECT_VIEWER_RCS})
QT4_WRAP_CPP( OBJECT_VIEWER_MOC_SRCS ${OBJECT_VIEWER_HDR} )

SOURCE_GROUP(QtResources FILES ${OBJECT_VIEWER_RCS})
SOURCE_GROUP(QtGeneratedMocQrcSrc FILES ${OBJECT_VIEWER_MOC_SRCS} ${OBJECT_VIEWER_RC_SRCS})

ADD_EXECUTABLE(object_viewer_qt WIN32 MACOSX_BUNDLE
               ${OBJECT_VIEWER_SRC}
               ${OBJECT_VIEWER_MOC_SRCS}
               ${OBJECT_VIEWER_RC_SRCS}
	       ${OBJECT_VIEWER_RC})

TARGET_LINK_LIBRARIES(object_viewer_qt
		nelmisc 
		${QT_LIBRARIES}
		${QT_QTMAIN_LIBRARY})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS ${LIBXML2_DEFINITIONS} ${QT_DEFINITIONS})
NL_DEFAULT_PROPS(object_viewer_qt "NeL, Tools, 3D: Object Viewer Qt")
NL_ADD_RUNTIME_FLAGS(object_viewer_qt)

ADD_SUBDIRECTORY(plugins)

INSTALL(TARGETS object_viewer_qt RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT runtime BUNDLE DESTINATION /Applications)
