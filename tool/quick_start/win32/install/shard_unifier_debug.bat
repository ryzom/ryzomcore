call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\shard_dev
set "PATH=%RC_SERVER_DIRS_DEBUG%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
call %RC_ROOT%\_servdash.bat start_unifier.ini
