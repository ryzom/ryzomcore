ADD_SUBDIRECTORY(land_export)
ADD_SUBDIRECTORY(land_export_lib)

IF(WITH_MFC)
  ADD_SUBDIRECTORY(world_editor)
  ADD_SUBDIRECTORY(world_editor_primitive_plugin)
ENDIF(WITH_MFC)

# This is an old plugin and is deprecated. It doesn't even compile anymore.
#ADD_SUBDIRECTORY(world_editor_plugin)
