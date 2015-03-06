
IF(WITH_NEL_TOOLS)
  IF(WITH_3D)
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
      shapes_exporter
      shape2obj
      zone_check_bind
      zone_dump
      zviewer)
  ENDIF()
    
  SUBDIRS(
    build_interface
    get_neighbors
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
      ENDIF(MAXSDK_FOUND)
    ENDIF(WITH_NEL_MAXPLUGIN)
  ENDIF(MFC_FOUND)
ENDIF()

IF(WITH_NEL_TOOLS AND WITH_3D)
  IF(WIN32)
#    ADD_SUBDIRECTORY(lightmap_optimizer)  
    IF(MFC_FOUND)
      ADD_SUBDIRECTORY(object_viewer_exe)
      ADD_SUBDIRECTORY(tile_edit)
    ENDIF(MFC_FOUND)
  ENDIF(WIN32)
  
  IF(WITH_QT)
    ADD_SUBDIRECTORY(tile_edit_qt)
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

ENDIF()

