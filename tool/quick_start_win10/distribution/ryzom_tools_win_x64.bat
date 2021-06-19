call ..\path_config.bat
rmdir /s /q ryzom_tools_win_x64
if %errorlevel% neq 0 pause
mkdir ryzom_tools_win_x64
cd ryzom_tools_win_x64

copy %RC_ROOT%\build\server_x64\bin\Release\ryzom_patchman_service.exe ryzom_patchman_service.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\sheets_packer.exe sheets_packer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\sheets_packer_shard.exe sheets_packer_shard.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\build_world_packed_col.exe build_world_packed_col.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\ai_build_wmap.exe ai_build_wmap.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\r2_islands_textures.exe r2_islands_textures.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\land_export.exe land_export.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\prim_export.exe prim_export.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\ryzom_mission_compiler.exe ryzom_mission_compiler.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\ryzom_mission_compiler_fe.exe ryzom_mission_compiler_fe.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\patch_gen_service.exe patch_gen_service.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\stats_scan.exe stats_scan.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\patch_gen.exe patch_gen.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\pdr_util.exe pdr_util.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\alias_synchronizer.exe alias_synchronizer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\translation_tools.exe translation_tools.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\uni_conv.exe uni_conv.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\make_alias_file.exe make_alias_file.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\skill_extractor.exe skill_extractor.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\assoc_mem.exe assoc_mem.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\pd_parser.exe pd_parser.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\7zDec.exe 7zDec.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\named2csv.exe named2csv.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\make_anim_by_race.exe make_anim_by_race.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\make_anim_melee_impact.exe make_anim_melee_impact.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\mp_generator.exe mp_generator.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\icon_search.exe icon_search.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\csv_transform.exe csv_transform.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\georges.exe georges.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\georges_dll_r.dll georges_dll_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\georges_plugin_sound_r.dll georges_plugin_sound_r.dll

copy %RC_ROOT%\build\tools_x64\bin\Release\world_editor.exe world_editor.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\world_editor_sound_plugin_r.dll world_editor_sound_plugin_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\world_editor_fauna_graph_plugin_r.dll world_editor_fauna_graph_plugin_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\world_editor_graph_plugin_r.dll world_editor_graph_plugin_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\world_editor_primitive_plugin_r.dll world_editor_primitive_plugin_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\world_editor_shard_monitor_plugin_r.dll world_editor_shard_monitor_plugin_r.dll

copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_direct3d_win_r.dll nel_drv_direct3d_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_opengl_win_r.dll nel_drv_opengl_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_openal_win_r.dll nel_drv_openal_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_fmod_win_r.dll nel_drv_fmod_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_xaudio2_win_r.dll nel_drv_xaudio2_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_dsound_win_r.dll nel_drv_dsound_win_r.dll

"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 B4D4201C74969879C6052F05631BEA4F5265BDEF /t http://timestamp.comodoca.com/authenticode "*.exe"
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 B4D4201C74969879C6052F05631BEA4F5265BDEF /t http://timestamp.comodoca.com/authenticode "*.dll"

copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\create_levedesign_archive.bat create_levedesign_archive.bat
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\create_levedesign_data_archive.bat create_levedesign_data_archive.bat
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\georges.cfg georges.cfg
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\history.txt history.txt
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ligoscape.cfg ligoscape.cfg
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\mission_compiler.cfg mission_compiler.cfg
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\mount_l.bat mount_l.bat
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\world_editor.html world_editor.html
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\world_editor_plugin.cfg world_editor_plugin.cfg
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\world_editor_script.xml world_editor_script.xml

mkdir ui
cd ui
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\actions.ico actions.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\alias.ico alias.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\audio.ico audio.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\audio_hiden.ico audio_hiden.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\bot_template_npc.ico bot_template_npc.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\bot_template_npc_ml.ico bot_template_npc_ml.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\bot_template_outpost.ico bot_template_outpost.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\bot_template_outpost_ml.ico bot_template_outpost_ml.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\building_destination.ico building_destination.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\building_instance.ico building_instance.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\building_template.ico building_template.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\building_trigger.ico building_trigger.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\cell.ico cell.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\cell_zone.ico cell_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\cell_zones.ico cell_zones.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\continent.ico continent.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\deposit.ico deposit.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\depositzone.ico depositzone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\dyn_answer.ico dyn_answer.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\dyn_fauna_zone.ico dyn_fauna_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\dyn_npc_zone.ico dyn_npc_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\dyn_road.ico dyn_road.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\dynamic_region.ico dynamic_region.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\dynamic_system.ico dynamic_system.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\emissary.ico emissary.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\env_fx.ico env_fx.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\env_fx_zone.ico env_fx_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\event.ico event.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\event_action.ico event_action.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\event_handler.ico event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\event_handler_action.ico event_handler_action.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\exclude.ico exclude.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\fauna.ico fauna.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\fauna_event_handler.ico fauna_event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\fauna_event_handler_action.ico fauna_event_handler_action.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\fauna_generic_place.ico fauna_generic_place.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\fauna_state.ico fauna_state.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\fauna_state_event_handler.ico fauna_state_event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flat_dyn_chat_continue.ico flat_dyn_chat_continue.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flat_dyn_chat_fail.ico flat_dyn_chat_fail.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flat_dyn_chat_retry.ico flat_dyn_chat_retry.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flat_dyn_chat_skippable.ico flat_dyn_chat_skippable.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flora.ico flora.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flora_exclude.ico flora_exclude.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flora_path.ico flora_path.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\flora_zone.ico flora_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\food.ico food.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\gear.ico gear.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\geom_items.ico geom_items.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group.ico group.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_descriptions.ico group_descriptions.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_fauna.ico group_fauna.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_fauna_ex.ico group_fauna_ex.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_template.ico group_template.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_template_fauna.ico group_template_fauna.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_template_npc.ico group_template_npc.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_template_npc_ml.ico group_template_npc_ml.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_template_outpost.ico group_template_outpost.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\group_template_outpost_ml.ico group_template_outpost_ml.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\handon.ico handon.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\jump_to.ico jump_to.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\kami_base.ico kami_base.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\kami_deposit.ico kami_deposit.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\kami_group.ico kami_group.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\kami_guardian.ico kami_guardian.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\kami_manager.ico kami_manager.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\kami_preacher.ico kami_preacher.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\karavan_base.ico karavan_base.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\karavan_emissary.ico karavan_emissary.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\karavan_group.ico karavan_group.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\karavan_guard.ico karavan_guard.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\karavan_manager.ico karavan_manager.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\linear_dyn_chat_continue.ico linear_dyn_chat_continue.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\linear_dyn_chat_fail.ico linear_dyn_chat_fail.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\linear_dyn_chat_retry.ico linear_dyn_chat_retry.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\linear_dyn_chat_skippable.ico linear_dyn_chat_skippable.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\manager.ico manager.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission.ico mission.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission_bot_chat_step.ico mission_bot_chat_step.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission_objectives.ico mission_objectives.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission_reward.ico mission_reward.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission_reward_group.ico mission_reward_group.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission_step.ico mission_step.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\mission_tree.ico mission_tree.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\missions_editor.ico missions_editor.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\no_answer.ico no_answer.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\no_go.ico no_go.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc.ico npc.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_bot.ico npc_bot.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_event_handler.ico npc_event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_event_handler_action.ico npc_event_handler_action.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_folder.ico npc_folder.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_group.ico npc_group.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_group_event_handler.ico npc_group_event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_manager.ico npc_manager.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_punctual_state.ico npc_punctual_state.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_route.ico npc_route.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_state_chat.ico npc_state_chat.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_state_event_handler.ico npc_state_event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_state_profile.ico npc_state_profile.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\npc_zone.ico npc_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\outpost.ico outpost.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\outpost_manager.ico outpost_manager.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\outpost_spawn_zone.ico outpost_spawn_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\path.ico path.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\people.ico people.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\place.ico place.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\point.ico point.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\population.ico population.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\preacher.ico preacher.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\raw_material_flora.ico raw_material_flora.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\raw_material_ground.ico raw_material_ground.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\raw_material_season.ico raw_material_season.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\region.ico region.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\respawn_point.ico respawn_point.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\respawn_points.ico respawn_points.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\rest.ico rest.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\result_no.ico result_no.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\result_yes.ico result_yes.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\room_destination.ico room_destination.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\room_template.ico room_template.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\sample_bank_zone.ico sample_bank_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\sample_banks.ico sample_banks.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\shield.ico shield.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\sound_path.ico sound_path.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\sound_point.ico sound_point.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\sound_zone.ico sound_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\sounds.ico sounds.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\spawn.ico spawn.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\spawn_base.ico spawn_base.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\stable.ico stable.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\stable_entry.ico stable_entry.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\stables.ico stables.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\state.ico state.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\state_event_handler.ico state_event_handler.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\state_machine.ico state_machine.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\state_machine_list.ico state_machine_list.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\step.ico step.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\step_any.ico step_any.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\step_dyn_chat.ico step_dyn_chat.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\step_failure.ico step_failure.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\step_if.ico step_if.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\step_ooo.ico step_ooo.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\teleport_dest.ico teleport_dest.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\teleport_destination.ico teleport_destination.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\teleport_dests.ico teleport_dests.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\teleport_spawn_zone.ico teleport_spawn_zone.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\teleport_trigger.ico teleport_trigger.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\temp.ico temp.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\time.ico time.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_creature.ico var_creature.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_faction.ico var_faction.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_item.ico var_item.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_npc.ico var_npc.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_place.ico var_place.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_quality.ico var_quality.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_quantity.ico var_quantity.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\var_text.ico var_text.ico
copy %RC_ROOT%\code\ryzom\tools\leveldesign\install\ui\zone.ico zone.ico
cd ..

copy "C:\2019q4_external_v142_x64\ogg\bin\ogg.dll" "ogg.dll"
copy "C:\2019q4_external_v142_x64\libpng\bin\libpng16.dll" "libpng16.dll"
copy "C:\2019q4_external_v142_x64\vorbis\bin\vorbis.dll" "vorbis.dll"
copy "C:\2019q4_external_v142_x64\vorbis\bin\vorbisfile.dll" "vorbisfile.dll"
copy "C:\2019q4_external_v142_x64\libjpeg\bin\jpeg62.dll" "jpeg62.dll"
copy "C:\2019q4_external_v142_x64\libxml2\bin\libxml2.dll" "libxml2.dll"
copy "C:\2019q4_external_v142_x64\libiconv\bin\libiconv.dll" "libiconv.dll"
copy "C:\2019q4_external_v142_x64\libiconv\bin\libcharset.dll" "libcharset.dll"
copy "C:\2019q4_external_v142_x64\zlib\bin\zlib.dll" "zlib.dll"
copy "C:\2019q4_external_v142_x64\freetype\bin\freetype.dll" "freetype.dll"
copy "C:\2019q4_external_v142_x64\openal-soft\bin\OpenAL32.dll" "OpenAL32.dll"
rem copy "C:\2019q4_external_v142_x64\openssl\bin\ssleay32.dll" "ssleay32.dll"
rem copy "C:\2019q4_external_v142_x64\openssl\bin\libeay32.dll" "libeay32.dll"
rem copy "C:\2019q4_external_v142_x64\curl\bin\libcurl.dll" "libcurl.dll"
copy "C:\2019q4_external_v142_x64\mariadb-connector-c\bin\libmariadb.dll" "libmariadb.dll"

copy "%RC_ROOT%\distribution\utils\lzma.exe" "lzma.exe"
copy "%RC_ROOT%\distribution\utils\rclone.exe" "rclone.exe"
copy "%RC_ROOT%\distribution\utils\xdelta.exe" "xdelta.exe"
copy "%RC_ROOT%\distribution\utils\7za.exe" "7za.exe"
copy "%RC_ROOT%\distribution\utils\nircmd.exe" "nircmd.exe"
copy "%RC_ROOT%\distribution\utils\msxsl.exe" "msxsl.exe"
copy "%RC_ROOT%\distribution\utils\servdash.exe" "servdash.exe"

copy C:\2019q4_external_v142_x64\qt5\bin\Qt5Core.dll Qt5Core.dll
copy C:\2019q4_external_v142_x64\qt5\bin\Qt5Gui.dll Qt5Gui.dll
copy C:\2019q4_external_v142_x64\qt5\bin\Qt5Test.dll Qt5Test.dll
copy C:\2019q4_external_v142_x64\qt5\bin\Qt5Xml.dll Qt5Xml.dll
copy C:\2019q4_external_v142_x64\qt5\bin\Qt5Widgets.dll Qt5Widgets.dll
copy C:\2019q4_external_v142_x64\qt5\bin\Qt5Network.dll Qt5Network.dll
copy C:\2019q4_external_v142_x64\qt5\bin\Qt5PrintSupport.dll Qt5PrintSupport.dll
copy C:\2019q4_external_v142_x64\qt5\bin\libGLESv2.dll libGLESv2.dll
copy C:\2019q4_external_v142_x64\qt5\bin\libEGL.dll libEGL.dll

rmdir /s /q platforms
mkdir platforms
cd platforms
copy C:\2019q4_external_v142_x64\qt5\plugins\platforms\qwindows.dll qwindows.dll
cd ..

rmdir /s /q styles
mkdir styles
cd styles
copy C:\2019q4_external_v142_x64\qt5\plugins\styles\qwindowsvistastyle.dll qwindowsvistastyle.dll
cd ..

rmdir /s /q imageformats
mkdir imageformats
cd imageformats
copy C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qjpeg.dll qjpeg.dll
copy C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qgif.dll qgif.dll
copy C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qtga.dll qtga.dll
copy C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qwbmp.dll qwbmp.dll
copy C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qtiff.dll qtiff.dll
copy C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qwebp.dll qwebp.dll
cd ..

cd ..

exit

