call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\client_dev
set "PATH=%RC_CLIENT_DIRS_RELEASE%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
rem The client listens for nel_mcp (_client_mcp.bat, .mcp.json) as client_dev, then client_devb... for more of them
rem What it shows comes from the local shard and your own accounts: trusted, so nel_mcp may run Lua and commands in it
start ryzom_client_dev_r.exe --ctrl-listen=client_dev# --ctrl-trusted
