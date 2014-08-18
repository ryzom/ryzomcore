INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
			${LIBXML2_INCLUDE_DIR}
			${QT_INCLUDES}
			${CMAKE_CURRENT_SOURCE_DIR}/scene/
			${CMAKE_CURRENT_SOURCE_DIR}/particle_system/
			${CMAKE_CURRENT_SOURCE_DIR}/vegetable/
			${CMAKE_CURRENT_SOURCE_DIR}/widgets/
)

FILE(GLOB SRC *.cpp *.h)

FILE(GLOB OVQT_SCENE_SRC      scene/*.cpp scene/*.h)
FILE(GLOB OVQT_PS_SRC         particle_system/*.h particle_system/*.cpp)
FILE(GLOB OVQT_VEGETABLE_SRC  vegetable/*.h vegetable/*.cpp)
FILE(GLOB OVQT_WIDGETS_SRC    widgets/*.h widgets/*.cpp)

SET(OVQT_EXT_SYS_SRC	${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_manager.h
			${CMAKE_CURRENT_SOURCE_DIR}/../../extension_system/iplugin_spec.h)

SET(OBJECT_VIEWER_PLUGIN_QT_HDR object_viewer_plugin.h
				main_window.h
				graphics_viewport.h
				graphics_settings_page.h
				sound_settings_page.h
				vegetable_settings_page.h

				scene/animation_dialog.h
				scene/animation_set_dialog.h
				scene/setup_fog_dialog.h
				scene/slot_manager_dialog.h
				scene/water_pool_dialog.h
				scene/skeleton_scale_dialog.h 
				scene/skeleton_tree_model.h 
				scene/global_wind_dialog.h
				scene/day_night_dialog.h
				scene/sun_color_dialog.h
				scene/tune_mrm_dialog.h
				scene/tune_timer_dialog.h
				scene/camera_control.h

				particle_system/particle_control_dialog.h
				particle_system/particle_workspace_dialog.h
				particle_system/particle_tree_model.h
				particle_system/particle_system_page.h
				particle_system/particle_workspace_page.h
				particle_system/emitter_page.h
				particle_system/attrib_widget.h
				particle_system/located_bindable_page.h
				particle_system/located_page.h
				particle_system/particle_force_page.h 
				particle_system/particle_light_page.h 
				particle_system/particle_zone_page.h 
				particle_system/particle_sound_page.h
				particle_system/basic_edit_widget.h 
				particle_system/direction_widget.h 
				particle_system/scheme_bank_dialog.h
				particle_system/particle_property_dialog.h
				particle_system/ps_mover_page.h
				particle_system/value_blender_dialog.h 
				particle_system/value_gradient_dialog.h
				particle_system/value_from_emitter_dialog.h 
				particle_system/curve_dialog.h
				particle_system/bin_op_dialog.h
				particle_system/mesh_widget.h
				particle_system/morph_mesh_dialog.h
				particle_system/constraint_mesh_widget.h
				particle_system/tail_particle_widget.h
				particle_system/auto_lod_dialog.h
				particle_system/particle_texture_widget.h
				particle_system/particle_texture_anim_widget.h
				particle_system/multi_tex_dialog.h
				particle_system/spinner_dialog.h
				particle_system/follow_path_dialog.h
				particle_system/particle_link_skeleton_dialog.h

				vegetable/vegetable_dialog.h
				vegetable/vegetable_noise_value_widget.h
				vegetable/vegetable_density_page.h
				vegetable/vegetable_landscape_page.h
				vegetable/vegetable_scale_page.h
				vegetable/vegetable_appearance_page.h
				vegetable/vegetable_rotate_page.h

				widgets/edit_range_widget.h
				widgets/color_edit_widget.h
				widgets/hoverpoints.h
)

SET(OBJECT_VIEWER_PLUGIN_UIS    scene/animation_form.ui
				scene/animation_set_form.ui
				scene/setup_fog_form.ui
				scene/slot_form.ui
				scene/skeleton_scale_form.ui
				scene/water_pool_form.ui
				scene/global_wind_form.ui
				scene/sun_color_form.ui
				scene/day_night_form.ui
				scene/tune_mrm_form.ui
				scene/tune_timer_form.ui
				
				particle_system/particle_control_form.ui
				particle_system/particle_workspace_form.ui
				particle_system/particle_system_form.ui
				particle_system/workspace_form.ui
				particle_system/attrib_form.ui
				particle_system/emitter_form.ui
				particle_system/located_bindable_form.ui
				particle_system/located_form.ui
				particle_system/particle_force_form.ui
				particle_system/particle_light_form.ui
				particle_system/particle_zone_form.ui
				particle_system/particle_sound_form.ui
				particle_system/basic_edit_form.ui
				particle_system/direction_form.ui
				particle_system/ps_mover_form.ui
				particle_system/curve_form.ui
				particle_system/mesh_form.ui
				particle_system/morph_mesh_form.ui
				particle_system/constraint_mesh_form.ui
				particle_system/tail_form.ui
				particle_system/auto_lod_form.ui
				particle_system/particle_texture_form.ui
				particle_system/particle_texture_anim_form.ui
				particle_system/multi_tex_form.ui				
				particle_system/particle_link_skeleton_form.ui
				particle_system/scheme_bank_form.ui
				particle_system/value_gradient_form.ui

				vegetable/vegetable_dialog_form.ui
				vegetable/vegetable_noise_value_form.ui
				vegetable/vegetable_density_form.ui
				vegetable/vegetable_apperance_form.ui
				vegetable/vegetable_landscape_form.ui
				vegetable/vegetable_rotate_form.ui
				vegetable/vegetable_scale_form.ui

				widgets/color_edit_form.ui
				widgets/edit_range_float_form.ui
				widgets/edit_range_uint_form.ui

				graphics_settings_page.ui
				sound_settings_page.ui
				vegetable_settings_page.ui
)

SET(OBJECT_VIEWER_PLUGIN_RCS object_viewer.qrc)

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

QT4_ADD_RESOURCES(OBJECT_VIEWER_PLUGIN_RC_SRCS ${OBJECT_VIEWER_PLUGIN_RCS})
QT4_WRAP_CPP(OBJECT_VIEWER_PLUGIN_MOC_SRC ${OBJECT_VIEWER_PLUGIN_QT_HDR})
QT4_WRAP_UI(OBJECT_VIEWER_PLUGIN_UI_HDRS ${OBJECT_VIEWER_PLUGIN_UIS})

SOURCE_GROUP(QtResources FILES ${OBJECT_VIEWER_PLUGIN_UIS})
SOURCE_GROUP(QtGeneratedUiHdr FILES ${OBJECT_VIEWER_PLUGIN_UI_HDRS})
SOURCE_GROUP(QtGeneratedMocSrc FILES ${OBJECT_VIEWER_PLUGIN_MOC_SRC})

SOURCE_GROUP("ovqt Extension System" FILES ${OVQT_EXT_SYS_SRC})
SOURCE_GROUP("ovqt Plugin Src" FILES ${SRC})
SOURCE_GROUP("ovqt Plugin Scene Src" FILES ${OVQT_SCENE_SRC})
SOURCE_GROUP("ovqt Plugin PS Src" FILES ${OVQT_PS_SRC})
SOURCE_GROUP("ovqt Plugin Vegetable Src" FILES ${OVQT_VEGETABLE_SRC})
SOURCE_GROUP("ovqt Plugin Widgets Src" FILES ${OVQT_WIDGETS_SRC})

ADD_LIBRARY(studio_plugin_object_viewer MODULE ${SRC} 
                                             ${OVQT_SCENE_SRC}
                                             ${OVQT_PS_SRC}
                                             ${OVQT_VEGETABLE_SRC}
                                             ${OVQT_WIDGETS_SRC}
                                             ${OBJECT_VIEWER_PLUGIN_MOC_SRC}
                                             ${OVQT_EXT_SYS_SRC}
                                             ${OBJECT_VIEWER_PLUGIN_UI_HDRS}
                                             ${OBJECT_VIEWER_PLUGIN_RC_SRCS})

TARGET_LINK_LIBRARIES(studio_plugin_object_viewer 
			studio_plugin_core 
			nelmisc 
			nel3d
			nelsound
			nelligo
			nelgeorges
			${QT_LIBRARIES}
			${QT_QTOPENGL_LIBRARY})

NL_DEFAULT_PROPS(studio_plugin_object_viewer "Tools: Studio Plugin: Object Viewer")

NL_ADD_RUNTIME_FLAGS(studio_plugin_object_viewer)
NL_ADD_LIB_SUFFIX(studio_plugin_object_viewer)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS ${LIBXML2_DEFINITIONS} -DQT_PLUGIN -DQT_SHARED ${QT_DEFINITIONS})

IF(WITH_PCH)
  ADD_NATIVE_PRECOMPILED_HEADER(studio_plugin_object_viewer ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp)
ENDIF(WITH_PCH)



IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF(WITH_INSTALL_LIBRARIES)
ELSE(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF(WITH_INSTALL_LIBRARIES)
ENDIF(WIN32)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_object_viewer.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

