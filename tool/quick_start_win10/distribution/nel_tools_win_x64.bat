call ..\path_config.bat
rmdir /s /q nel_tools_win_x64
if %errorlevel% neq 0 pause
mkdir nel_tools_win_x64
cd nel_tools_win_x64

rem ???
copy %RC_ROOT%\build\tools_x64\bin\Release\crash_log_analyser.exe crash_log_analyser.exe

rem copy %RC_ROOT%\build\tools_x64\bin\Release\branch_patcher.exe branch_patcher.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\message_box.exe message_box.exe
rem copy %RC_ROOT%\build\tools_x64\bin\Release\multi_cd_setup_fix.exe multi_cd_setup_fix.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\file_info.exe file_info.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\anim_builder.exe anim_builder.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\animation_set_builder.exe animation_set_builder.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_clod_bank.exe build_clod_bank.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_clodtex.exe build_clodtex.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_coarse_mesh.exe build_coarse_mesh.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_far_bank.exe build_far_bank.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_interface.exe build_interface.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\unbuild_interface.exe unbuild_interface.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_shadow_skin.exe build_shadow_skin.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_smallbank.exe build_smallbank.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\get_neighbors.exe get_neighbors.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\hls_bank_maker.exe hls_bank_maker.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\ig_add.exe ig_add.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\ig_elevation.exe ig_elevation.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\ig_info.exe ig_info.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\ig_lighter.exe ig_lighter.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\lightmap_optimizer.exe lightmap_optimizer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\panoply_maker.exe panoply_maker.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\tga2dds.exe tga2dds.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\tga_cut.exe tga_cut.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\tga_resize.exe tga_resize.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\tile_edit.exe tile_edit.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_check_bind.exe zone_check_bind.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_dependencies.exe zone_dependencies.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_dump.exe zone_dump.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_ig_lighter.exe zone_ig_lighter.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_lighter.exe zone_lighter.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_welder.exe zone_welder.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zone_elevation.exe zone_elevation.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\memlog.exe memlog.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\bnp_make.exe bnp_make.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\snp_make.exe snp_make.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\data_mirror.exe data_mirror.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\disp_sheet_id.exe disp_sheet_id.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\exec_timeout.exe exec_timeout.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\extract_filename.exe extract_filename.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\lock.exe lock.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\log_analyser.exe log_analyser.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\extract_warnings_r.dll extract_warnings_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\make_sheet_id.exe make_sheet_id.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\words_dic.exe words_dic.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\xml_packer.exe xml_packer.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\build_ig_boxes.exe build_ig_boxes.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_indoor_rbank.exe build_indoor_rbank.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_rbank.exe build_rbank.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\georges2csv.exe georges2csv.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\build_sound.exe build_sound.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_soundbank.exe build_soundbank.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\build_samplebank.exe build_samplebank.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\cluster_viewer.exe cluster_viewer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\zviewer.exe zviewer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\object_viewer.exe object_viewer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\object_viewer_dll_r.dll object_viewer_dll_r.dll

copy %RC_ROOT%\build\tools_x64\bin\Release\logic_editor.exe logic_editor.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\logic_editor_dll_r.dll logic_editor_dll_r.dll

copy %RC_ROOT%\build\tools_x64\bin\Release\mesh_export.exe mesh_export.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\shapes_exporter.exe shapes_exporter.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\shape2obj.exe shape2obj.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\nl_probe_timers.exe nl_probe_timers.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\textures_optimizer.exe textures_optimizer.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\textures_tool.exe textures_tool.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\nl_panoply_preview.exe nl_panoply_preview.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\crash_report.exe crash_report.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\tile_edit_qt.exe tile_edit_qt.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\words_dic_qt.exe words_dic_qt.exe
copy %RC_ROOT%\build\tools_x64\bin\Release\message_box_qt.exe message_box_qt.exe

copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_direct3d_win_r.dll nel_drv_direct3d_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_opengl_win_r.dll nel_drv_opengl_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_openal_win_r.dll nel_drv_openal_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_fmod_win_r.dll nel_drv_fmod_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_xaudio2_win_r.dll nel_drv_xaudio2_win_r.dll
copy %RC_ROOT%\build\tools_x64\bin\Release\nel_drv_dsound_win_r.dll nel_drv_dsound_win_r.dll

"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 B4D4201C74969879C6052F05631BEA4F5265BDEF /t http://timestamp.comodoca.com/authenticode "*.exe"
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 B4D4201C74969879C6052F05631BEA4F5265BDEF /t http://timestamp.comodoca.com/authenticode "*.dll"

copy %RC_ROOT%\code\nel\tools\3d\build_clod_bank\config_example.cfg build_clod_bank_config_example.cfg
copy %RC_ROOT%\code\nel\tools\3d\build_clod_bank\path_config_example.cfg build_clod_bank_path_config_example.cfg
copy %RC_ROOT%\code\nel\tools\3d\build_coarse_mesh\build.cfg build_coarse_mesh.cfg
copy %RC_ROOT%\code\nel\tools\pacs\build_ig_boxes\build_ig_boxes.cfg build_ig_boxes.cfg
copy %RC_ROOT%\code\nel\tools\pacs\build_indoor_rbank\build_indoor_rbank.cfg build_indoor_rbank.cfg
copy %RC_ROOT%\code\nel\tools\pacs\build_rbank\build_rbank.cfg build_rbank.cfg
copy %RC_ROOT%\code\nel\tools\misc\data_mirror\config.cfg data_mirror.cfg
copy %RC_ROOT%\code\nel\tools\3d\ig_lighter\config.cfg ig_lighter.cfg
copy %RC_ROOT%\code\nel\tools\misc\make_sheet_id\make_sheet_id.cfg make_sheet_id.cfg
copy %RC_ROOT%\code\nel\tools\3d\object_viewer\object_viewer.cfg object_viewer.cfg
copy %RC_ROOT%\code\nel\tools\3d\panoply_maker\panoply.cfg panoply.cfg
copy %RC_ROOT%\code\nel\tools\misc\words_dic\words_dic.cfg words_dic.cfg
copy %RC_ROOT%\code\nel\tools\3d\zone_lighter\zone_lighter.cfg zone_lighter.cfg
copy %RC_ROOT%\code\nel\tools\3d\zone_welder\zwelder.cfg zwelder.cfg
copy %RC_ROOT%\code\nel\tools\3d\zviewer\zviewer.cfg zviewer.cfg

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
copy "C:\2019q4_external_v142_x64\assimp\bin\assimp.dll" "assimp.dll"
rem copy "C:\2019q4_external_v142_x64\openssl\bin\ssleay32.dll" "ssleay32.dll"
rem copy "C:\2019q4_external_v142_x64\openssl\bin\libeay32.dll" "libeay32.dll"
rem copy "C:\2019q4_external_v142_x64\curl\bin\libcurl.dll" "libcurl.dll"

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

