call %~dp0_r_check.bat
set "RC_TOOLS_DIRS=%RC_TOOLS_DIRS_RELEASE%"
set "PATH=%RC_PATH%;%RC_TOOLS_DIRS%;%RC_EXTERNAL_BIN_DIRS%;%RC_PYTHON27_DIR%;%PATH%"
set "QT_PLUGIN_DIR=%RC_QT_PLUGIN_DIR%"
cd /d %RC_CODE_DIR%\nel\tools\build_gamedata
dir *.bat
dir *.py
cmd
