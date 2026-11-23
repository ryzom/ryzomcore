@echo off
rem The Model Context Protocol server of the service dashboard, for an agent such as Claude
rem Code: .mcp.json in this directory starts it. It serves every dashboard running for this
rem user (the shards opened with shard_*.bat, web_on.bat) and can start one from its ini in
rem pipeline\shard_dev; servdash_gui or servdash_cli, as _servdash.bat chooses. The dashboards
rem keep running when the agent leaves.
call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\shard_dev
set "PATH=%RC_SERVER_DIRS_RELEASE%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
call %RC_ROOT%\_servdash.bat --mcp %*
