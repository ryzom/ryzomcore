INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${LIBXML2_INCLUDE_DIR}
			${QT_INCLUDES})

FILE(GLOB SRC *.cpp *.h)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OBJECT_VIEWER_PLUGIN_HDR 	object_viewer_plugin.h
				main_window.h 
				graphics_viewport.h 
				animation_dialog.h 
				animation_set_dialog.h 
				setup_fog_dialog.h 
				slot_manager_dialog.h 
				particle_control_dialog.h 
				particle_workspace_dialog.h
				particle_tree_model.h 
				particle_system_page.h 
				particle_workspace_page.h 
				edit_range_widget.h
				emitter_page.h 
				attrib_widget.h 
				located_bindable_page.h 
				located_page.h
				particle_force_page.h 
				particle_light_page.h 
				particle_zone_page.h 
				particle_sound_page.h
				basic_edit_widget.h 
				direction_widget.h 
				color_edit_widget.h 
				particle_property_dialog.h
				ps_mover_page.h
				value_blender_dialog.h 
				value_gradient_dialog.h
				value_from_emitter_dialog.h 
				curve_dialog.h
				bin_op_dialog.h
				hoverpoints.h
				mesh_widget.h
				morph_mesh_dialog.h
				constraint_mesh_widget.h
				tail_particle_widget.h
				auto_lod_dialog.h
				particle_texture_widget.h
				particle_texture_anim_widget.h
				multi_tex_dialog.h
				spinner_dialog.h
				follow_path_dialog.h
				water_pool_dialog.h
				skeleton_scale_dialog.h 
				skeleton_tree_model.h 
				particle_link_skeleton_dialog.h
				vegetable_dialog.h
				global_wind_dialog.h
				day_night_dialog.h
				sun_color_dialog.h
				vegetable_noise_value_widget.h
				vegetable_density_page.h
				vegetable_landscape_page.h
				vegetable_scale_page.h
				vegetable_appearance_page.h
				vegetable_rotate_page.h
				tune_mrm_dialog.h
				tune_timer_dialog.h
				camera_control.h
				graphics_settings_page.h
				sound_settings_page.h
				vegetable_settings_page.h
				scheme_bank_dialog.h)

SET(OBJECT_VIEWER_PLUGIN_UIS    animation_form.ui
				animation_set_form.ui
				setup_fog_form.ui
				slot_form.ui
				particle_control_form.ui
				particle_workspace_form.ui
				edit_range_float_form.ui
				edit_range_uint_form.ui
				particle_system_form.ui
				workspace_form.ui
				attrib_form.ui
				emitter_form.ui
				located_bindable_form.ui
				located_form.ui
				particle_force_form.ui
				particle_light_form.ui
				particle_zone_form.ui
				particle_sound_form.ui
				basic_edit_form.ui
				direction_form.ui
				color_edit_form.ui
				ps_mover_form.ui
				curve_form.ui
				mesh_form.ui
				morph_mesh_form.ui
				constraint_mesh_form.ui
				tail_form.ui
				auto_lod_form.ui
				particle_texture_form.ui
				particle_texture_anim_form.ui
				multi_tex_form.ui
				skeleton_scale_form.ui
				particle_link_skeleton_form.ui
				water_pool_form.ui
				vegetable_dialog_form.ui
				vegetable_noise_value_form.ui
				global_wind_form.ui
				sun_color_form.ui
				day_night_form.ui
				vegetable_density_form.ui
				vegetable_apperance_form.ui
				vegetable_landscape_form.ui
				vegetable_rotate_form.ui
				vegetable_scale_form.ui
				tune_mrm_form.ui
				tune_timer_form.ui
				graphics_settings_page.ui
				sound_settings_page.ui
				vegetable_settings_page.ui
				scheme_bank_form.ui
				value_gradient_form.ui)

SET(OBJECT_VIEWER_PLUGIN_RCS object_viewer.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_ADD_RESOURCES(OBJECT_VIEWER_PLUGIN_RC_SRCS ${OBJECT_VIEWER_PLUGIN_RCS})
QT4_WRAP_CPP(OBJECT_VIEWER_PLUGIN_MOC_SRC ${OBJECT_VIEWER_PLUGIN_HDR})
QT4_WRAP_UI(OBJECT_VIEWER_PLUGIN_UI_HDRS ${OBJECT_VIEWER_PLUGIN_UIS})

SOURCE_GROUP(QtResources FILES ${OBJECT_VIEWER_PLUGIN_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OBJECT_VIEWER_PLUGIN_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OBJECT_VIEWER_PLUGIN_MOC_SRC})
SOURCE_GROUP("Object Viewer Plugin" FILES ${SRC})
SOURCE_GROUP("OVQT Extension System" FILES ${OVQT_EXT_SYS_SRC})

ADD_LIBRARY(ovqt_plugin_object_viewer MODULE ${SRC} ${OBJECT_VIEWER_PLUGIN_MOC_SRC} ${OVQT_EXT_SYS_SRC} ${OBJECT_VIEWER_PLUGIN_UI_HDRS} ${OBJECT_VIEWER_PLUGIN_RC_SRCS})

TARGET_LINK_LIBRARIES(ovqt_plugin_object_viewer 
			ovqt_plugin_core 
			nelmisc 
			nel3d
			nelsound
			nelligo
			nelgeorges
			${QT_LIBRARIES}
			${QT_QTOPENGL_LIBRARY})

NL_DEFAULT_PROPS(ovqt_plugin_object_viewer "NeL, Tools, 3D: Object Viewer Qt Plugin: Object Viewer")

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(ovqt_plugin_object_viewer ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)

NL_ADD_RUNTIME_FLAGS(ovqt_plugin_object_viewer)
NL_ADD_LIB_SUFFIX(ovqt_plugin_object_viewer)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS ${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

INSTALL(TARGETS ovqt_plugin_object_viewer LIBRARY DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION bin ARCHIVE DESTINATION ${NL_LIB_PREFIX} COMPONENT tools3d)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.xml DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
