ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(example)
ADD_SUBDIRECTORY(ovqt_sheet_builder)
ADD_SUBDIRECTORY(landscape_editor)
ADD_SUBDIRECTORY(log)
ADD_SUBDIRECTORY(disp_sheet_id)
ADD_SUBDIRECTORY(object_viewer)
ADD_SUBDIRECTORY(georges_editor)

IF(WITH_GUI)
  ADD_SUBDIRECTORY(gui_editor)
ENDIF(WITH_GUI)

ADD_SUBDIRECTORY(translation_manager)
ADD_SUBDIRECTORY(bnp_manager)
# Note: Temporarily disabled until development continues.
#ADD_SUBDIRECTORY(zone_painter)
# Ryzom Specific Plugins
IF(WITH_RYZOM AND WITH_RYZOM_TOOLS)
  ADD_SUBDIRECTORY(mission_compiler)
ENDIF(WITH_RYZOM AND WITH_RYZOM_TOOLS)
