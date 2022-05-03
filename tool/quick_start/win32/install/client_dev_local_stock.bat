call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\client_dev
set "PATH=%RC_CLIENT_DIRS_STOCK%;%PATH%"
start ryzom_client_dev_r.exe
