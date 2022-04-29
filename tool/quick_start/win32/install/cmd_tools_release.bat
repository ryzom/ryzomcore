call %~dp0_r_check.bat
set "RC_TOOLS_DIRS=%RC_TOOLS_DIRS_RELEASE%"
set "RC_SAMPLES_DIRS=%RC_SAMPLES_DIRS_RELEASE%"
set "PATH=%RC_PATH%;%RC_TOOLS_DIRS%;%RC_SAMPLES_DIRS%;%RC_EXTERNAL_BIN_DIRS%;%PATH%"
cd /d %RC_ROOT%\.nel\tools
dir %RC_ROOT%\build_win32\tools\bin\Release\*.exe
cmd
