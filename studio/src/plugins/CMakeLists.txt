ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(example)
ADD_SUBDIRECTORY(ovqt_sheet_builder)
ADD_SUBDIRECTORY(landscape_editor)
ADD_SUBDIRECTORY(log)
ADD_SUBDIRECTORY(disp_sheet_id)
ADD_SUBDIRECTORY(object_viewer)
ADD_SUBDIRECTORY(georges_editor)

ADD_SUBDIRECTORY(world_editor)
IF(WITH_GUI)
  ADD_SUBDIRECTORY(gui_editor)
ENDIF()

# translation_manager uploads translations through QFtp, which Qt 5 dropped from
# QtNetwork (only the separate qtftp add-on has it, and no distribution ships that).
IF(WITH_QT5)
  MESSAGE(STATUS "studio: translation_manager plugin skipped - it needs QFtp, which Qt 5 no longer has")
ELSE()
  ADD_SUBDIRECTORY(translation_manager)
ENDIF()
ADD_SUBDIRECTORY(tile_editor)
ADD_SUBDIRECTORY(bnp_manager)
# Note: Temporarily disabled until development continues.
#ADD_SUBDIRECTORY(zone_painter)

# Ryzom Specific Plugins
IF(WITH_RYZOM AND WITH_RYZOM_TOOLS)
  ADD_SUBDIRECTORY(mission_compiler)
ENDIF()
