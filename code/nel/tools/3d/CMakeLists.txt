
IF(WITH_NEL_TOOLS)

  SUBDIRS(
    build_coarse_mesh
    build_far_bank
    build_smallbank
    ig_lighter
    zone_dependencies
    zone_ig_lighter 
    zone_lighter
    zone_welder
    animation_set_builder
    anim_builder
    build_clod_bank
    build_clodtex
    build_interface
    build_shadow_skin
    cluster_viewer
    file_info
    get_neighbors
    ig_add
    ig_info
    shapes_exporter
    tga_cut
    tga_resize
    shape2obj
    zone_check_bind
    zone_dump
    zviewer)

ENDIF(WITH_NEL_TOOLS)

# For tools selection of only max plugins
IF(WIN32)  
  IF(MFC_FOUND)
    ADD_SUBDIRECTORY(object_viewer)
    IF(WITH_NEL_MAXPLUGIN)
      IF(MAXSDK_FOUND)
        ADD_SUBDIRECTORY(plugin_max)
        ADD_SUBDIRECTORY(ligo)
      ENDIF(MAXSDK_FOUND)
    ENDIF(WITH_NEL_MAXPLUGIN)
  ENDIF(MFC_FOUND)
ENDIF(WIN32)

IF(WITH_NEL_TOOLS)

  IF(WIN32)
    ADD_SUBDIRECTORY(ig_elevation)
    ADD_SUBDIRECTORY(lightmap_optimizer)  
    IF(MFC_FOUND)
      ADD_SUBDIRECTORY(object_viewer_exe)
      ADD_SUBDIRECTORY(tile_edit)
    ENDIF(MFC_FOUND)
  ENDIF(WIN32)
  
  IF(WITH_QT)
    ADD_SUBDIRECTORY(tile_edit_qt)
    ADD_SUBDIRECTORY(object_viewer_qt)
    ADD_SUBDIRECTORY(object_viewer_widget)
  ENDIF(WITH_QT)
  
  IF(SQUISH_FOUND)
    ADD_SUBDIRECTORY(s3tc_compressor_lib)
    ADD_SUBDIRECTORY(panoply_maker)
    ADD_SUBDIRECTORY(tga_2_dds)
    ADD_SUBDIRECTORY(hls_bank_maker)
  ENDIF(SQUISH_FOUND)

  #crash_log_analyser
  #shapes_exporter

ENDIF(WITH_NEL_TOOLS)

