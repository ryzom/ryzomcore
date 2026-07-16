INCLUDE_DIRECTORIES(	${CMAKE_CURRENT_BINARY_DIR}
			${CMAKE_CURRENT_SOURCE_DIR}
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

SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTOPENGL TRUE)

SOURCE_GROUP("ovqt Extension System" FILES ${OVQT_EXT_SYS_SRC})
SOURCE_GROUP("ovqt Plugin Src" FILES ${SRC})
SOURCE_GROUP("ovqt Plugin Scene Src" FILES ${OVQT_SCENE_SRC})
SOURCE_GROUP("ovqt Plugin PS Src" FILES ${OVQT_PS_SRC})
SOURCE_GROUP("ovqt Plugin Vegetable Src" FILES ${OVQT_VEGETABLE_SRC})
SOURCE_GROUP("ovqt Plugin Widgets Src" FILES ${OVQT_WIDGETS_SRC})

ADD_LIBRARY(studio_plugin_object_viewer MODULE ${OVQT_EXT_SYS_SRC})

target_sources(studio_plugin_object_viewer
        PRIVATE
        graphics_settings_page.cpp
        graphics_viewport.cpp
        main_window.cpp
        modules.cpp
        object_viewer.cpp
        object_viewer_plugin.cpp
        particle_system/attrib_widget.cpp
        particle_system/auto_lod_dialog.cpp
        particle_system/basic_edit_widget.cpp
        particle_system/bin_op_dialog.cpp
        particle_system/constraint_mesh_widget.cpp
        particle_system/curve_dialog.cpp
        particle_system/direction_widget.cpp
        particle_system/dup_ps.cpp
        particle_system/emitter_page.cpp
        particle_system/follow_path_dialog.cpp
        particle_system/located_bindable_page.cpp
        particle_system/located_page.cpp
        particle_system/mesh_widget.cpp
        particle_system/morph_mesh_dialog.cpp
        particle_system/multi_tex_dialog.cpp
        particle_system/particle_control_dialog.cpp
        particle_system/particle_editor.cpp
        particle_system/particle_force_page.cpp
        particle_system/particle_light_page.cpp
        particle_system/particle_link_skeleton_dialog.cpp
        particle_system/particle_node.cpp
        particle_system/particle_property_dialog.cpp
        particle_system/particle_sound_page.cpp
        particle_system/particle_system_page.cpp
        particle_system/particle_texture_anim_widget.cpp
        particle_system/particle_texture_widget.cpp
        particle_system/particle_tree_model.cpp
        particle_system/particle_workspace_dialog.cpp
        particle_system/particle_workspace_page.cpp
        particle_system/particle_zone_page.cpp
        particle_system/ps_initial_pos.cpp
        particle_system/ps_mover_page.cpp
        particle_system/scheme_bank_dialog.cpp
        particle_system/scheme_manager.cpp
        particle_system/spinner_dialog.cpp
        particle_system/tail_particle_widget.cpp
        particle_system/value_blender_dialog.cpp
        particle_system/value_from_emitter_dialog.cpp
        particle_system/value_gradient_dialog.cpp
        scene/animation_dialog.cpp
        scene/animation_set_dialog.cpp
        scene/camera_control.cpp
        scene/day_night_dialog.cpp
        scene/entity.cpp
        scene/global_wind_dialog.cpp
        scene/setup_fog_dialog.cpp
        scene/skeleton_scale_dialog.cpp
        scene/skeleton_tree_model.cpp
        scene/slot_manager_dialog.cpp
        scene/sun_color_dialog.cpp
        scene/tune_mrm_dialog.cpp
        scene/tune_timer_dialog.cpp
        scene/water_pool_dialog.cpp
        sound_settings_page.cpp
        sound_system.cpp
        stdpch.cpp
        vegetable/vegetable_appearance_page.cpp
        vegetable/vegetable_density_page.cpp
        vegetable/vegetable_dialog.cpp
        vegetable/vegetable_editor.cpp
        vegetable/vegetable_landscape_page.cpp
        vegetable/vegetable_node.cpp
        vegetable/vegetable_noise_value_widget.cpp
        vegetable/vegetable_rotate_page.cpp
        vegetable/vegetable_scale_page.cpp
        vegetable_settings_page.cpp
        widgets/color_edit_widget.cpp
        widgets/edit_range_widget.cpp
        widgets/hoverpoints.cpp
        object_viewer.qrc
        graphics_settings_page.ui
        particle_system/attrib_form.ui
        particle_system/auto_lod_form.ui
        particle_system/basic_edit_form.ui
        particle_system/constraint_mesh_form.ui
        particle_system/curve_form.ui
        particle_system/direction_form.ui
        particle_system/emitter_form.ui
        particle_system/located_bindable_form.ui
        particle_system/located_form.ui
        particle_system/mesh_form.ui
        particle_system/morph_mesh_form.ui
        particle_system/multi_tex_form.ui
        particle_system/particle_control_form.ui
        particle_system/particle_force_form.ui
        particle_system/particle_light_form.ui
        particle_system/particle_link_skeleton_form.ui
        particle_system/particle_sound_form.ui
        particle_system/particle_system_form.ui
        particle_system/particle_texture_anim_form.ui
        particle_system/particle_texture_form.ui
        particle_system/particle_workspace_form.ui
        particle_system/particle_zone_form.ui
        particle_system/ps_mover_form.ui
        particle_system/scheme_bank_form.ui
        particle_system/tail_form.ui
        particle_system/value_gradient_form.ui
        particle_system/workspace_form.ui
        scene/animation_form.ui
        scene/animation_set_form.ui
        scene/day_night_form.ui
        scene/global_wind_form.ui
        scene/setup_fog_form.ui
        scene/skeleton_scale_form.ui
        scene/slot_form.ui
        scene/sun_color_form.ui
        scene/tune_mrm_form.ui
        scene/tune_timer_form.ui
        scene/water_pool_form.ui
        sound_settings_page.ui
        vegetable/vegetable_apperance_form.ui
        vegetable/vegetable_density_form.ui
        vegetable/vegetable_dialog_form.ui
        vegetable/vegetable_landscape_form.ui
        vegetable/vegetable_noise_value_form.ui
        vegetable/vegetable_rotate_form.ui
        vegetable/vegetable_scale_form.ui
        vegetable_settings_page.ui
        widgets/color_edit_form.ui
        widgets/edit_range_float_form.ui
        widgets/edit_range_uint_form.ui
)

TARGET_LINK_LIBRARIES(studio_plugin_object_viewer
        PRIVATE
		Studio::plugin::core
		NeL::misc
		NeL::3d
		NeL::sound
		NeL::ligo
		NeL::georges
		Qt::Core
		Qt::Gui
		Qt::Widgets
)

NL_DEFAULT_PROPS(studio_plugin_object_viewer "Tools: Studio Plugin: Object Viewer")

NL_ADD_RUNTIME_FLAGS(studio_plugin_object_viewer)
NL_ADD_LIB_SUFFIX(studio_plugin_object_viewer)

TARGET_COMPILE_DEFINITIONS(studio_plugin_object_viewer PRIVATE QT_NO_KEYWORDS QT_PLUGIN QT_SHARED ${QT_DEFINITIONS})

IF(WITH_PCH)
  target_precompile_headers(studio_plugin_object_viewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h)
ENDIF()



IF(WIN32)
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${OVQT_PLUGIN_DIR} COMPONENT tools3d)
  ENDIF()
ELSE()
  IF(WITH_INSTALL_LIBRARIES)
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} ARCHIVE DESTINATION ${NL_LIB_PREFIX} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ELSE()
    INSTALL(TARGETS studio_plugin_object_viewer LIBRARY DESTINATION ${OVQT_PLUGIN_DIR} RUNTIME DESTINATION ${NL_BIN_PREFIX} COMPONENT tools3d)
  ENDIF()
ENDIF()

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/studio_plugin_object_viewer.xml DESTINATION ${OVQT_PLUGIN_SPECS_DIR} COMPONENT tools3d)

