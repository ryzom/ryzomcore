@echo off
rem Opens the service dashboard on the shard ini given, from the shard directory.
rem servdash_gui (built with the tools) opens in its own window. With "Headless": true in
rem .nel\config.json, servdash_cli (built with the cross-platform tools, build_docker\tools_xplat)
rem runs in this console instead, for a box
rem reached over SSH: F10 detaches and the services keep running, the same command attaches
rem again, and `servdash_cli --list` names every dashboard running.
rem The Win32 servdash of the externals is the fallback when neither is found.
rem With --mcp as the first argument the chosen tool runs in this console as the Model Context
rem Protocol server (see _servdash_mcp.bat and .mcp.json), which the fallback cannot do.
setlocal
if "%RC_HEADLESS%"=="1" goto :cli
set "PATH=%RC_TOOLS_DIRS_RELEASE%;%PATH%"
set "QT_PLUGIN_PATH=%RC_QT_PLUGIN_DIR%"
where /q servdash_gui
if %errorlevel% neq 0 goto :legacy
if "%~1"=="--mcp" (
	servdash_gui %*
	goto :eof
)
start servdash_gui %*
goto :eof
:cli
set "PATH=%RC_TOOLS_XPLAT_DIR_WIN%;%PATH%"
where /q servdash_cli
if %errorlevel% neq 0 goto :legacy
servdash_cli %*
goto :eof
:legacy
if "%~1"=="--mcp" (
	echo servdash_gui and servdash_cli were not found, the Win32 servdash has no MCP server 1>&2
	exit /b 1
)
echo servdash_gui and servdash_cli were not found, using the Win32 servdash
start %RC_ROOT%\external\utils\servdash %*
