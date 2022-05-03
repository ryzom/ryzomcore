call %~dp0_r_check.bat
set "RC_TOOLS_DIRS=%RC_TOOLS_DIRS_STOCK%%"
set "RC_SAMPLES_DIRS=%RC_SAMPLES_DIRS_STOCK%%"
set "PATH=%RC_PATH%;%RC_TOOLS_DIRS%;%RC_SAMPLES_DIRS%;%PATH%"
cd /d %RC_ROOT%\.nel\tools
dir %RC_ROOT%\stock\nel_tools_win_x64\*.exe
dir %RC_ROOT%\stock\ryzom_tools_win_x64\*.exe
cmd
