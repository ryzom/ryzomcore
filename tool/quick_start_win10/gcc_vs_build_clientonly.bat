call path_config.bat
cd /d %RC_ROOT%
cd build
call patch_version.bat
cmd /C "call vs_build_dev"
cmd /C "call vs_build_fv_x64"
cd ..
pause
