if /I "%RC_PYTHON2%"=="" call ..\path_config.bat
if /I "%CLIENT_PATCH_VERSION%"=="" goto :update
goto :done
:update
%RC_PYTHON2%\python %RC_ROOT%\build\patch_version.py
call patch_version_set.bat
:done
