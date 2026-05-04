#ADD_SUBDIRECTORY(3rdparty)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDES})

CONFIGURE_FILE(translations/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc COPYONLY)

FILE(GLOB STUDIO_SRC extension_system/*.h
							extension_system/*.cpp
							*.h *.cpp)

SET(STUDIO_HDR   extension_system/iplugin_manager.h
						extension_system/plugin_manager.h
						startup_settings_dlg.h
						splash_screen.h
						pm_watcher.h )

SET(STUDIO_RCS studio.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

SET(STUDIO_TS    translations/object_viewer_qt_en.ts
						translations/object_viewer_qt_fr.ts
						translations/object_viewer_qt_de.ts
						translations/object_viewer_qt_ru.ts)

SET(STUDIO_PLUGIN_UIS startup_settings_dlg.ui )

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

IF(WIN32)
  SET(STUDIO_RC studio.rc)
ENDIF()

QT5_ADD_TRANSLATION(STUDIO_QM ${STUDIO_TS})

SOURCE_GROUP(QtResources FILES ${STUDIO_RCS} ${STUDIO_PLUGIN_UIS} )

ADD_EXECUTABLE(studio WIN32 MACOSX_BUNDLE)

target_sources(studio
		PRIVATE
		${STUDIO_SRC}
		startup_settings_dlg.ui
		studio.qrc
)

IF(WIN32)
	target_sources(studio
			PRIVATE
			studio.rc
	)
ENDIF()



TARGET_LINK_LIBRARIES(studio
		PRIVATE
		NeL::misc
		Qt::Core
		Qt::Widgets
)

TARGET_COMPILE_DEFINITIONS(studio PRIVATE QT_NO_KEYWORDS ${QT_DEFINITIONS})
NL_DEFAULT_PROPS(studio "Tools: Studio")
NL_ADD_RUNTIME_FLAGS(studio)

ADD_SUBDIRECTORY(libqtpropertybrowser-text)
ADD_SUBDIRECTORY(plugins)

add_dependencies(studio
		studio_plugin_bnp_manager
		studio_plugin_core
		studio_plugin_disp_sheet_id
		studio_plugin_example
		studio_plugin_georges_editor
		studio_plugin_landscape_editor
		studio_plugin_log
		studio_plugin_object_viewer
		studio_plugin_sheet_builder
		studio_plugin_tile_editor
#		studio_plugin_translation_manager # requires QFtp which is removed in qt5
		studio_plugin_world_editor
#		studio_plugin_zone_painter
)
IF(WITH_GUI)
	add_dependencies(studio
			studio_plugin_gui_editor
	)
ENDIF()
IF(WITH_RYZOM AND WITH_RYZOM_TOOLS)
	add_dependencies(studio
			studio_plugin_mission_compiler
	)
ENDIF()

INSTALL(TARGETS studio RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT runtime BUNDLE DESTINATION /Applications)
