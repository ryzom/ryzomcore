call ..\path_config.bat
rmdir /s /q nel_plugins_3dsmax_2019_x64
if %errorlevel% neq 0 pause
mkdir nel_plugins_3dsmax_2019_x64
cd nel_plugins_3dsmax_2019_x64
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\object_viewer_dll_r.dll" "object_viewer_dll_r.dll"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_3dsmax_shared_r.dll" "nel_3dsmax_shared_r.dll"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_drv_opengl_win_r.dll" "nel_drv_opengl_win_r.dll"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_drv_xaudio2_win_r.dll" "nel_drv_xaudio2_win_r.dll"
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 B4D4201C74969879C6052F05631BEA4F5265BDEF /t http://timestamp.comodoca.com/authenticode "*.dll"
copy "C:\2019q4_external_v142_x64\ogg\bin\ogg.dll" "ogg.dll"
rem copy "C:\2019q4_external_v142_x64\libpng\bin\libpng16.dll" "libpng16.dll"
copy "C:\2019q4_external_v142_x64\vorbis\bin\vorbis.dll" "vorbis.dll"
copy "C:\2019q4_external_v142_x64\vorbis\bin\vorbisfile.dll" "vorbisfile.dll"
rem copy "C:\2019q4_external_v142_x64\libjpeg\bin\jpeg62.dll" "jpeg62.dll"
copy "C:\2019q4_external_v142_x64\libxml2\bin\libxml2.dll" "libxml2.dll"
copy "C:\2019q4_external_v142_x64\libiconv\bin\libiconv.dll" "libiconv.dll"
copy "C:\2019q4_external_v142_x64\libiconv\bin\libcharset.dll" "libcharset.dll"
copy "C:\2019q4_external_v142_x64\zlib\bin\zlib.dll" "zlib.dll"
copy "C:\2019q4_external_v142_x64\freetype\bin\freetype.dll" "freetype.dll"
mkdir plugins
cd plugins
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_patch_converter_r.dlm" "nelconvertpatch_r.dlm"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_export_r.dlu" "nelexport_r.dlu"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_patch_paint_r.dlm" "nelpaintpatch_r.dlm"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_patch_edit_r.dlm" "neleditpatch_r.dlm"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\tile_utility_r.dlu" "neltileutility_r.dlu"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\nel_vertex_tree_paint_r.dlm" "nel_vertex_tree_paint_r.dlm"
copy "%RC_ROOT%\build\3dsmax\2019_x64\bin\Release\ligoscape_utility_r.dlx" "nelligoscapeutility_r.dlx"
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 B4D4201C74969879C6052F05631BEA4F5265BDEF /t http://timestamp.comodoca.com/authenticode "*.*"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\nel_patch_paint\keys.cfg" "keys.cfg"
copy "%RC_ROOT%\code\nel\tools\3d\ligo\ligoscape.cfg" "ligoscape.cfg"
cd ..
rem xcopy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC" ".\"
rem xcopy "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT" ".\"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_animation_support.txt" "max_animation_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_light_support.txt" "max_light_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_lightmap_support.txt" "max_lightmap_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_material_support.txt" "max_material_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_skinning_support.txt" "max_skinning_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\nel_water_material.txt" "nel_water_material.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\resolve_troubles.txt" "resolve_troubles.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_animation_support.txt" "max_animation_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\max_animation_support.txt" "max_animation_support.txt"
copy "%RC_ROOT%\code\nel\tools\3d\object_viewer\object_viewer.cfg" "object_viewer.cfg"
mkdir scripts
cd scripts
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\db_cleaner.ms" "db_cleaner.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\db_erase_mesh.ms" "db_erase_mesh.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\db_shooter.ms" "db_shooter.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\extrude_water.ms" "extrude_water.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_add_name_ref_scale.ms" "nel_add_name_ref_scale.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_assets_png.ms" "nel_assets_png.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_assets_png_batched.ms" "nel_assets_png_batched.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_assets_png_database.ms" "nel_assets_png_database.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_batched_mergesave.ms" "nel_batched_mergesave.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_batched_script.ms" "nel_batched_script.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_copy_biped_figure_mode.ms" "nel_copy_biped_figure_mode.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_create_matrix.ms" "nel_create_matrix.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_mat_converter.ms" "nel_mat_converter.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_mirror_weights.ms" "nel_mirror_weights.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_move_animation.ms" "nel_move_animation.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_node_properties.ms" "nel_node_properties.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_old_zone_to_ligo.ms" "nel_old_zone_to_ligo.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_rename.ms" "nel_rename.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_repair_xref.ms" "nel_repair_xref.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_select.ms" "nel_select.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_select_ig.ms" "nel_select_ig.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_utility.ms" "nel_utility.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\nel_xref_building.ms" "nel_xref_building.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\reload_textures.ms" "reload_textures.ms"
copy "%RC_ROOT%\code\nel\tools\3d\ligo\plugin_max\scripts\nel_ligoscape.ms" "nel_ligoscape.ms"
mkdir startup
cd startup
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_flare.ms" "nel_flare.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_light.ms" "nel_light.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_material.ms" "nel_material.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_material.ms.v1" "nel_material.ms.v1"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_material.ms.v2" "nel_material.ms.v2"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_material.ms.v3" "nel_material.ms.v3"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_material.ms.v5" "nel_material.ms.v5"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_material.ms.v11" "nel_material.ms.v11"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_multi_set.ms" "nel_multi_set.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_pacs_box.ms" "nel_pacs_box.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_pacs_cylinder.ms" "nel_pacs_cylinder.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_ps.ms" "nel_ps.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_swt.ms" "nel_swt.ms"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\scripts\startup\nel_wave_maker.ms" "nel_wave_maker.ms"
cd ..
cd ..
mkdir macroscripts
cd macroscripts
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\macroscripts\nel_mirror_weights.mcr" "nel_mirror_weights.mcr"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\macroscripts\nel_node_properties.mcr" "nel_node_properties.mcr"
copy "%RC_ROOT%\code\nel\tools\3d\plugin_max\macroscripts\nel_xref_building.mcr" "nel_xref_building.mcr"
copy "%RC_ROOT%\code\nel\tools\3d\ligo\plugin_max\macroscripts\nel_ligoscape.mcr" "nel_ligoscape.mcr"
cd ..
cd ..

exit

