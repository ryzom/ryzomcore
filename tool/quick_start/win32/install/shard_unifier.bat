call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\shard_dev
set "PATH=%RC_SERVER_DIRS_RELEASE%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
start %RC_ROOT%\external\utils\servdash start_unifier.ini
