@echo off
rem The Model Context Protocol server of the clients, nel_mcp, for an agent such as Claude Code:
rem .mcp.json in this directory starts it. It serves every client with a control endpoint running
rem for this user, such as the ones client_dev.bat and client_dev_live.bat open (client_dev, then
rem client_devb...): their state, log and commands, their screen, and the methods that drive them.
rem It launches clients itself only with a profile (nel_mcp --profile=<file>), see the client control
rem page of the wiki. It is the nel_mcp of the client build, never another one on PATH; not having
rem built it is an error: build the client first (code_build_game_dev.bat). Its log and its crash
rem reports go to pipeline\client_dev, beside the logs of the clients.
call %~dp0.nel\path_config.bat
set "PATH=%RC_CLIENT_DIRS_RELEASE%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
set "NEL_MCP_EXE="
for %%d in ("%RC_CLIENT_DIRS_RELEASE:;=" "%") do if not defined NEL_MCP_EXE if exist "%%~d\nel_mcp.exe" set "NEL_MCP_EXE=%%~d\nel_mcp.exe"
if not defined NEL_MCP_EXE goto missing
"%NEL_MCP_EXE%" "--log-dir=%RC_ROOT%\pipeline\client_dev" %*
exit /b %errorlevel%
:missing
>&2 echo nel_mcp.exe is in none of %RC_CLIENT_DIRS_RELEASE%: build the client first (code_build_game_dev.bat)
exit /b 1
