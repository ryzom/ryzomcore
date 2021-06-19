call path_config.bat
cd /d %RC_ROOT%
set "MSBUILDEXTRA=/t:Clean;Rebuild"
cd build
call patch_version.bat
cmd /C "call vs_build_dev"
cmd /C "call vs_build_fv_x64"
cmd /C "call vs_build_fv_x86"
cmd /C "call vs_build_tools"
cmd /C "call vs_build_samples"
cmd /C "call vs_build_server"
cmd /C "call vs_build_3dsmax_x86"
cmd /C "call vs_build_3dsmax_x64"
cmd /C "call gcc_build"
cd ..
pause
