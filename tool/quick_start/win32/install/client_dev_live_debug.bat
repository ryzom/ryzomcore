call %~dp0.nel\path_config.bat
cd /d %RC_ROOT%\pipeline\client_dev_live
set "PATH=%RC_CLIENT_DIRS_DEBUG%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
rem The client listens for nel_mcp (_client_mcp.bat, .mcp.json) as client_dev_live, then client_dev_liveb... for more of them
rem What it shows is written by the players and web pages of the public shard: untrusted, so no --ctrl-trusted here
start ryzom_client_dev_d.exe --ctrl-listen=client_dev_live#
