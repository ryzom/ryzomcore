@echo off
title Ryzom Core Configuration
powershell ";"
echo -------
echo --- Ryzom Core Configuration
echo -------
echo(
echo This script will set up the buildsite configuration, and create needed directories.
echo To use the defaults, simply hit ENTER, else type in the new value.
echo Use -- if you need to insert an empty value.
echo(
echo Optionally requires Visual Studio 2019, and WSL 1 with Ubuntu 18.04 LTS and GCC 8.
echo(
goto :config
:baddir
echo ERROR: Do not run this script from R:
echo(
pause
exit
:config
if exist path_config.bat call path_config.bat
set RC_ROOT_TEMP=%~dp0
set RC_ROOT_TEMP=%RC_ROOT_TEMP:~0,-1%
if /I "%RC_ROOT_TEMP%"=="R:" goto :baddir
set RC_ROOT=%RC_ROOT_TEMP%
for /f "delims=" %%A in ('cd') do (
set RC_ROOT_NAME=%%~nxA
)
if not exist %RC_ROOT%\external\python27\python.exe goto :userpython
set RC_PYTHON2=%RC_ROOT%\external\python27
goto :apply
:userpython
set RC_PYTHON2_TEMP=
set /p RC_PYTHON2_TEMP=Python 2.7 (%RC_PYTHON2%): 
if /I "%RC_PYTHON2_TEMP%"=="" set RC_PYTHON2_TEMP=%RC_PYTHON2%
if /I "%RC_PYTHON2_TEMP%"=="--" set RC_PYTHON2_TEMP=
set RC_PYTHON2=%RC_PYTHON2_TEMP%
if not exist %RC_PYTHON2%\python.exe goto :config
echo(
:apply
set PATH=%RC_PYTHON2%;%PATH%
(
echo set RC_PYTHON2=%RC_PYTHON2%
echo set RC_ROOT=%RC_ROOT%
)>path_config.bat
echo Mounting %RC_ROOT% as R:
call _r_init.bat
rem cd /d R:\
echo | set /p=Updating references
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\tile_edit.lnk');$s.TargetPath='%RC_ROOT%\distribution\nel_tools_win_x64\tile_edit.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\nel_tools_win_x64\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\object_viewer.lnk');$s.TargetPath='%RC_ROOT%\distribution\nel_tools_win_x64\object_viewer.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\nel_tools_win_x64\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\georges.lnk');$s.TargetPath='%RC_ROOT%\distribution\ryzom_tools_win_x64\georges.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\ryzom_tools_win_x64\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\world_editor.lnk');$s.TargetPath='%RC_ROOT%\distribution\ryzom_tools_win_x64\world_editor.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\ryzom_tools_win_x64\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\build_gamedata.lnk');$s.TargetPath='%RC_ROOT%\code\nel\tools\build_gamedata';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\client_install_playing.lnk');$s.TargetPath='%RC_ROOT%\pipeline\client_install_playing';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\%RC_ROOT_NAME%.lnk');$s.TargetPath='%RC_ROOT%\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\R.lnk');$s.TargetPath='R:\';$s.Save()"
echo .
echo(
rem %RC_PYTHON2%\python .\code\configure\configure.py
rem echo(
cd /d %RC_ROOT%\code\nel\tools\build_gamedata
python 0_setup.py -p
if %errorlevel% neq 0 pause
if exist %RC_ROOT%\pipeline\install\data_leveldesign\sheet_id.bin goto :skipbuild
python a1_worldedit_data.py
if %errorlevel% neq 0 pause
python 1_export.py -ipj common/gamedev common/data_common common/sound common/leveldesign common/exedll shard/data_language shard/data_leveldesign shard/data_shard
if %errorlevel% neq 0 pause
python 2_build.py -ipj common/gamedev common/data_common common/sound common/leveldesign common/exedll shard/data_language shard/data_leveldesign shard/data_shard
if %errorlevel% neq 0 pause
cd /d %RC_ROOT%
call copy_dds_to_interfaces.bat
cd /d %RC_ROOT%\code\nel\tools\build_gamedata
python 3_install.py
if %errorlevel% neq 0 pause
:skipbuild
python b1_client_dev.py
if %errorlevel% neq 0 pause
python b2_shard_data.py
if %errorlevel% neq 0 pause
python b3_shard_dev.py
if %errorlevel% neq 0 pause
echo(
pause
