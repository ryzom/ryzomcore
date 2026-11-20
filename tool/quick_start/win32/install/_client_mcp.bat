@echo off
rem The Model Context Protocol server of the clients, nel_mcp, for an agent such as Claude Code:
rem .mcp.json in this directory starts it. It serves every client with a control endpoint running
rem for this user, such as the ones client_dev.bat and client_dev_live.bat open (client_dev, then
rem client_devb...): their state, log and commands, their screen, and the methods that drive them.
rem It launches clients itself only with a profile (nel_mcp --profile=<file>), see the client control
rem page of the wiki. Not having built it is an error: build the client first (code_build_game_dev.bat).
call %~dp0.nel\path_config.bat
set "PATH=%RC_CLIENT_DIRS_RELEASE%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
where /q nel_mcp
if %errorlevel% neq 0 (
	>&2 echo nel_mcp is not built: build the client first ^(code_build_game_dev.bat^)
	exit /b 1
)
nel_mcp %*
