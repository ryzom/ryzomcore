
IF(WITH_NEL_TOOLS)
  IF(WITH_3D)
    IF(WITH_ASSIMP)
      SUBDIRS(
        mesh_utils
        mesh_export)
    ENDIF()
    IF(WITH_LIBGSF)
      SUBDIRS(
        pipeline_max
        pipeline_max_dump
        pipeline_max_rewrite_assets)
    ENDIF()
    SUBDIRS(
      anim_builder
      animation_set_builder
      build_clod_bank
      build_clodtex
      build_coarse_mesh
      build_far_bank
      build_shadow_skin
      build_smallbank
      cluster_viewer
      file_info
      ig_add
      ig_elevation
      ig_info
      ig_lighter
      lightmap_optimizer
      zone_dependencies
      zone_ig_lighter
      zone_lighter
      zone_welder
      unbuild_elevation
      zone_elevation
      shapes_exporter
      shape2obj
      zone_check_bind
      zone_dump
      zviewer)
  ENDIF()
  SUBDIRS(
    build_interface
    unbuild_interface
    get_neighbors
    textures_optimizer
    textures_tool
    tga_cut
    tga_resize)
ENDIF()

# For tools selection of only max plugins
IF(WIN32 AND WITH_3D)
  IF(MFC_FOUND)
    ADD_SUBDIRECTORY(object_viewer)
    IF(WITH_NEL_MAXPLUGIN)
      IF(MAXSDK_FOUND)
        ADD_SUBDIRECTORY(plugin_max)
        ADD_SUBDIRECTORY(ligo)
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

IF(WITH_NEL_TOOLS AND WITH_3D)
  IF(WIN32)
#    ADD_SUBDIRECTORY(lightmap_optimizer)
    IF(MFC_FOUND)
      ADD_SUBDIRECTORY(object_viewer_exe)
      ADD_SUBDIRECTORY(tile_edit)
    ENDIF()
  ENDIF()

  IF(WITH_QT OR WITH_QT5)
    ADD_SUBDIRECTORY(tile_edit_qt)
    ADD_SUBDIRECTORY(object_viewer_widget)
  ENDIF()

  IF(WITH_QT5)
    ADD_SUBDIRECTORY(shared_widgets)
    ADD_SUBDIRECTORY(panoply_preview)
  ENDIF()

  IF(WITH_NEL_TOOLS)
    FIND_PACKAGE(Squish)
  ENDIF()

  IF(SQUISH_FOUND)
    ADD_SUBDIRECTORY(s3tc_compressor_lib)
    ADD_SUBDIRECTORY(panoply_maker)
    ADD_SUBDIRECTORY(tga_2_dds)
    ADD_SUBDIRECTORY(hls_bank_maker)
  ENDIF()

  #crash_log_analyser
ENDIF()

