ADD_SUBDIRECTORY(land_export)
ADD_SUBDIRECTORY(land_export_lib)

IF(WITH_MFC)
  ADD_SUBDIRECTORY(world_editor)
  ADD_SUBDIRECTORY(world_editor_fauna_graph_plugin)
  ADD_SUBDIRECTORY(world_editor_graph_plugin)
  ADD_SUBDIRECTORY(world_editor_primitive_plugin)
  ADD_SUBDIRECTORY(world_editor_shard_monitor_plugin)
  ADD_SUBDIRECTORY(world_editor_sound_plugin)
ENDIF(WITH_MFC)

# This is an old plugin and is deprecated. It doesn't even compile anymore.
#ADD_SUBDIRECTORY(world_editor_plugin)
