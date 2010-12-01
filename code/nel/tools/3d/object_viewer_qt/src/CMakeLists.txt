INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${LIBXML2_INCLUDE_DIR} ${NEL_INCLUDE_DIR} ${QT_INCLUDES})
INCLUDE( ${QT_USE_FILE} )

FILE(GLOB OBJECT_VIEWER_SRC configuration.h entity.h object_viewer.h particle_editor.h modules.h sound_system.h
			    particle_node.h ps_initial_pos.h dup_ps.h vegetable_editor.h vegetable_node.h 
			    extension_system/plugin_spec.h
			    extension_system/*.cpp *.cpp)
 
SET(OBJECT_VIEWER_HDR main_window.h graphics_viewport.h animation_dialog.h 
		      animation_set_dialog.h settings_dialog.h setup_fog_dialog.h 
		      slot_manager_dialog.h particle_control_dialog.h particle_workspace_dialog.h
		      particle_tree_model.h particle_system_page.h particle_workspace_page.h edit_range_widget.h
		      emitter_page.h attrib_widget.h located_bindable_page.h located_page.h
		      particle_force_page.h particle_light_page.h particle_zone_page.h particle_sound_page.h
		      basic_edit_widget.h direction_widget.h color_edit_widget.h particle_property_dialog.h
		      ps_mover_page.h graphics_info_widget.h value_blender_dialog.h value_gradient_dialog.h
		      value_from_emitter_dialog.h curve_dialog.h bin_op_dialog.h hoverpoints.h
		      mesh_widget.h morph_mesh_dialog.h constraint_mesh_widget.h tail_particle_widget.h
		      auto_lod_dialog.h particle_texture_widget.h particle_texture_anim_widget.h
		      multi_tex_dialog.h spinner_dialog.h follow_path_dialog.h water_pool_dialog.h
		      skeleton_scale_dialog.h skeleton_tree_model.h particle_link_skeleton_dialog.h
		      vegetable_dialog.h global_wind_dialog.h day_night_dialog.h sun_color_dialog.h
		      vegetable_noise_value_widget.h vegetable_density_page.h vegetable_landscape_page.h
		      vegetable_scale_page.h vegetable_appearance_page.h vegetable_rotate_page.h
		      extension_system/iplugin_manager.h extension_system/plugin_manager.h)
		      
SET(OBJECT_VIEWER_UIS animation_form.ui animation_set_form.ui settings_form.ui 
		      setup_fog_form.ui slot_form.ui particle_control_form.ui particle_workspace_form.ui
		      edit_range_float_form.ui edit_range_uint_form.ui particle_system_form.ui workspace_form.ui
		      attrib_form.ui emitter_form.ui located_bindable_form.ui located_form.ui
		      particle_force_form.ui particle_light_form.ui particle_zone_form.ui particle_sound_form.ui
		      basic_edit_form.ui direction_form.ui color_edit_form.ui ps_mover_form.ui curve_form.ui
		      mesh_form.ui morph_mesh_form.ui constraint_mesh_form.ui tail_form.ui auto_lod_form.ui
		      particle_texture_form.ui particle_texture_anim_form.ui multi_tex_form.ui skeleton_scale_form.ui
		      particle_link_skeleton_form.ui water_pool_form.ui vegetable_dialog_form.ui
		      vegetable_noise_value_form.ui global_wind_form.ui sun_color_form.ui day_night_form.ui
		      vegetable_density_form.ui vegetable_apperance_form.ui vegetable_landscape_form.ui
		      vegetable_rotate_form.ui vegetable_scale_form.ui)
SET(OBJECT_VIEWER_RCS object_viewer_qt.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_ADD_RESOURCES( OBJECT_VIEWER_RC_SRCS ${OBJECT_VIEWER_RCS} )
QT4_WRAP_CPP( OBJECT_VIEWER_MOC_SRCS ${OBJECT_VIEWER_HDR} )
QT4_WRAP_UI( OBJECT_VIEWER_UI_HDRS ${OBJECT_VIEWER_UIS} )

SOURCE_GROUP(QtResources FILES ${OBJECT_VIEWER_UIS} ${OBJECT_VIEWER_RCS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OBJECT_VIEWER_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OBJECT_VIEWER_MOC_SRCS})

ADD_EXECUTABLE(object_viewer_qt WIN32 MACOSX_BUNDLE ${OBJECT_VIEWER_SRC} ${OBJECT_VIEWER_MOC_SRCS} ${OBJECT_VIEWER_RC_SRCS} ${OBJECT_VIEWER_UI_HDRS})

TARGET_LINK_LIBRARIES(object_viewer_qt
		nelmisc 
		nel3d
		nelsound
		nelligo
		nelgeorges
		${QT_LIBRARIES}
		${QT_QTOPENGL_LIBRARY}
		${QT_QTMAIN_LIBRARY})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS ${LIBXML2_DEFINITIONS} ${QT_DEFINITIONS})
NL_DEFAULT_PROPS(object_viewer_qt "NeL, Tools, 3D: Object Viewer Qt")
NL_ADD_RUNTIME_FLAGS(object_viewer_qt)

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(object_viewer_qt ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

ADD_SUBDIRECTORY(plugins)

INSTALL(TARGETS object_viewer_qt RUNTIME DESTINATION bin COMPONENT runtime BUNDLE DESTINATION /Applications)
