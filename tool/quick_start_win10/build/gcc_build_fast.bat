call ..\path_config.bat
cd /d %RC_ROOT%\build
call patch_version.bat
bash -c "CLIENT_PATCH_VERSION=%CLIENT_PATCH_VERSION% SERVER_PATCH_VERSION=%SERVER_PATCH_VERSION% bash gcc_build_fast.sh"
pause
