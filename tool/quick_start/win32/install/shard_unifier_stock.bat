call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\shard_dev
set "PATH=%RC_SERVER_DIRS_STOCK%;%PATH%"
start %RC_ROOT%\external\utils\servdash start_unifier.ini
