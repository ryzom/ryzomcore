ADD_SUBDIRECTORY(3rdparty)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR} ${QT_INCLUDES})
INCLUDE( ${QT_USE_FILE} )

CONFIGURE_FILE(translations/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc COPYONLY)

FILE(GLOB STUDIO_SRC extension_system/*.h
							extension_system/*.cpp 
							*.h *.cpp)

SET(STUDIO_HDR   extension_system/iplugin_manager.h 
						extension_system/plugin_manager.h)

SET(STUDIO_RCS studio.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

SET(STUDIO_TS    translations/object_viewer_qt_en.ts
						translations/object_viewer_qt_fr.ts
						translations/object_viewer_qt_de.ts
						translations/object_viewer_qt_ru.ts)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

IF(WIN32)
  SET(STUDIO_RC studio.rc)
ENDIF(WIN32)

QT4_ADD_TRANSLATION(STUDIO_QM ${STUDIO_TS})
QT4_ADD_RESOURCES( STUDIO_RC_SRCS ${STUDIO_RCS})
QT4_WRAP_CPP( STUDIO_MOC_SRCS ${STUDIO_HDR} )

SOURCE_GROUP(QtResources FILES ${STUDIO_RCS})
SOURCE_GROUP(QtGeneratedMocQrcSrc FILES ${STUDIO_MOC_SRCS} ${STUDIO_RC_SRCS})

ADD_EXECUTABLE(studio WIN32 MACOSX_BUNDLE
               ${STUDIO_SRC}
               ${STUDIO_MOC_SRCS}
               ${STUDIO_RC_SRCS}
	       ${STUDIO_RC})

TARGET_LINK_LIBRARIES(studio
		nelmisc 
		${QT_LIBRARIES}
		${QT_QTMAIN_LIBRARY})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS ${LIBXML2_DEFINITIONS} ${QT_DEFINITIONS})
NL_DEFAULT_PROPS(studio "Tools: Studio")
NL_ADD_RUNTIME_FLAGS(studio)

ADD_SUBDIRECTORY(plugins)

INSTALL(TARGETS studio RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT runtime BUNDLE DESTINATION /Applications)
