@echo off
title Ryzom Core Configuration
powershell ";"
echo -------
echo --- Ryzom Core Configuration
echo -------
echo(
echo This script will set up the buildsite configuration, and create needed directories.
echo(
goto :config
:baddir
echo ERROR: Do not run this script from R:
echo(
pause
exit
:config
if exist .nel\path_config.bat call .nel\path_config.bat
mkdir .nel > nul 2> nul
set RC_ROOT_TEMP=%CD%
if /I "%RC_ROOT_TEMP%"=="R:" goto :baddir
set RC_ROOT=%RC_ROOT_TEMP%
for /f "delims=" %%A in ('cd') do (
set RC_ROOT_NAME=%%~nxA
)
if not exist %RC_ROOT%\external\python27\python.exe goto :userpython27
set RC_PYTHON27_DIR=%RC_ROOT%\external\python27
goto :checkpython3
:userpython27
echo To use the defaults, simply hit ENTER, else type in the new value.
echo Use -- if you need to insert an empty value.
set RC_PYTHON27_DIR_TEMP=
set /p RC_PYTHON27_DIR_TEMP=Python 2.7 (%RC_PYTHON27_DIR%): 
if /I "%RC_PYTHON27_DIR_TEMP%"=="" set RC_PYTHON27_DIR_TEMP=%RC_PYTHON27_DIR%
if /I "%RC_PYTHON27_DIR_TEMP%"=="--" set RC_PYTHON27_DIR_TEMP=
set RC_PYTHON27_DIR=%RC_PYTHON27_DIR_TEMP%
if not exist %RC_PYTHON27_DIR%\python.exe goto :config
echo(
:checkpython3
if not exist %RC_ROOT%\external\python3\python.exe goto :userpython3
set RC_PYTHON3_DIR=%RC_ROOT%\external\python3
goto :apply
:userpython3
echo To use the defaults, simply hit ENTER, else type in the new value.
echo Use -- if you need to insert an empty value.
set RC_PYTHON3_DIR_TEMP=
set /p RC_PYTHON3_DIR_TEMP=Python 3 (%RC_PYTHON3_DIR%): 
if /I "%RC_PYTHON3_DIR_TEMP%"=="" set RC_PYTHON3_DIR_TEMP=%RC_PYTHON3_DIR%
if /I "%RC_PYTHON3_DIR_TEMP%"=="--" set RC_PYTHON3_DIR_TEMP=
set RC_PYTHON3_DIR=%RC_PYTHON3_DIR_TEMP%
if not exist %RC_PYTHON3_DIR%\python.exe goto :config
echo(
:apply
set RC_ORIG_PATH=%PATH%
set PATH=%RC_PYTHON3_DIR%;%RC_ORIG_PATH%
cd /d %RC_ROOT%\code\tool\quick_start
python check_docker.py
if %errorlevel% neq 0 pause
python configure_toolchains.py
if %errorlevel% neq 0 pause
python configure_targets.py
if %errorlevel% neq 0 pause
cd /d %RC_ROOT%
echo(
xcopy %RC_ROOT%\code\tool\quick_start\win32\install\*.bat %RC_ROOT%\ /Y
if %errorlevel% neq 0 pause
mkdir %RC_ROOT%\external > nul 2> nul
xcopy %RC_ROOT%\code\tool\quick_start\win32\install\external\* %RC_ROOT%\external\ /Y
if %errorlevel% neq 0 pause
mkdir %RC_ROOT%\stock > nul 2> nul
xcopy %RC_ROOT%\code\tool\quick_start\win32\install\stock\*.bat %RC_ROOT%\stock\ /Y
if %errorlevel% neq 0 pause
mkdir %RC_ROOT%\.nel\tools > nul 2> nul
xcopy %RC_ROOT%\code\nel\tools\3d\object_viewer\*.cfg %RC_ROOT%\.nel\tools\ /Y
xcopy %RC_ROOT%\code\nel\tools\3d\zviewer\*.cfg %RC_ROOT%\.nel\tools\ /Y
xcopy %RC_ROOT%\code\ryzom\tools\leveldesign\install\*.cfg %RC_ROOT%\.nel\tools\ /Y
xcopy %RC_ROOT%\code\ryzom\tools\leveldesign\install\*.xml %RC_ROOT%\.nel\tools\ /Y
if %errorlevel% neq 0 pause
echo(
call .nel\path_config.bat
echo Mounting %RC_ROOT% as R:
call _r_check.bat
cd /d R:\
:lookfortoolsstock
set PATH=%RC_TOOLS_DIRS_STOCK%;%RC_ORIG_PATH%
where /q ryzom_patchman_service
if %errorlevel% neq 0 goto :notoolsstock
where /q sheets_packer_shard
if %errorlevel% neq 0 goto :notoolsstock
where /q panoply_maker
if %errorlevel% neq 0 goto :notoolsstock
:hastoolsstock
set RC_TOOLS_DIRS=%RC_TOOLS_DIRS_STOCK%
goto :lookfortoolsbuild
:notoolsstock
:lookfortoolsbuild
set PATH=%RC_TOOLS_DIRS_RELEASE%;%RC_ORIG_PATH%
where /q ryzom_patchman_service
if %errorlevel% neq 0 goto :notoolsbuild
where /q sheets_packer_shard
if %errorlevel% neq 0 goto :notoolsbuild
where /q panoply_maker
if %errorlevel% neq 0 goto :notoolsbuild
:hastoolsbuild
echo Using locally built tools
set RC_TOOLS_DIRS=%RC_TOOLS_DIRS_RELEASE%
set RC_ORIG_PATH=%RC_EXTERNAL_BIN_DIRS%;%RC_ORIG_PATH%
:notoolsbuild
if not defined RC_TOOLS_DIRS (
echo ERROR: Tools not found. Run `code_configure_rebuild_all` to build everything, and re-run the configuration script.
pause
exit
)
echo | set /p=Updating references
rem powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\tile_edit.lnk');$s.TargetPath='%RC_ROOT%\distribution\nel_tools_win_x64\tile_edit.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\nel_tools_win_x64\';$s.Save()"
rem echo | set /p=.
rem powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\object_viewer.lnk');$s.TargetPath='%RC_ROOT%\distribution\nel_tools_win_x64\object_viewer.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\nel_tools_win_x64\';$s.Save()"
rem echo | set /p=.
rem powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\georges.lnk');$s.TargetPath='%RC_ROOT%\distribution\ryzom_tools_win_x64\georges.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\ryzom_tools_win_x64\';$s.Save()"
rem echo | set /p=.
rem powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\world_editor.lnk');$s.TargetPath='%RC_ROOT%\distribution\ryzom_tools_win_x64\world_editor.exe';$s.WorkingDirectory='%RC_ROOT%\distribution\ryzom_tools_win_x64\';$s.Save()"
rem echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\build_gamedata.lnk');$s.TargetPath='%RC_ROOT%\code\nel\tools\build_gamedata';$s.Save()"
echo | set /p=.
rem powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\client_install_playing.lnk');$s.TargetPath='%RC_ROOT%\pipeline\client_install_playing';$s.Save()"
rem echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\%RC_ROOT_NAME%.lnk');$s.TargetPath='%RC_ROOT%\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\R.lnk');$s.TargetPath='R:\';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\localhost_phpmyadmin.lnk');$s.TargetPath='http://localhost:9042/phpmyadmin/';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\localhost_admin.lnk');$s.TargetPath='http://localhost:9042/admin/';$s.Save()"
echo | set /p=.
powershell "$s=(New-Object -COM WScript.Shell).CreateShortcut('%RC_ROOT%\localhost_ams.lnk');$s.TargetPath='http://localhost:9042/ams/';$s.Save()"
echo | set /p=.
echo .
echo(
set PATH=%RC_PYTHON27_DIR%;%RC_ORIG_PATH%
cd /d %RC_ROOT%\code\nel\tools\build_gamedata
python 0_setup.py -p
if %errorlevel% neq 0 pause
if exist %RC_ROOT%\pipeline\install\data_leveldesign\sheet_id.bin goto :skipbuild
python a1_worldedit_data.py
if %errorlevel% neq 0 pause
python 1_export.py -ipj common/gamedev common/data_common common/sound common/leveldesign common/exedll %RC_EXEDLL_PROJECTS% shard/data_language shard/data_leveldesign shard/data_shard
if %errorlevel% neq 0 pause
python 2_build.py -ipj common/gamedev common/data_common common/sound common/leveldesign common/exedll %RC_EXEDLL_PROJECTS% shard/data_language shard/data_leveldesign shard/data_shard
if %errorlevel% neq 0 pause
python 3_install.py
if %errorlevel% neq 0 pause
:skipbuild
cd /d %RC_ROOT%
call copy_dds_to_interfaces.bat
cd /d %RC_ROOT%\code\nel\tools\build_gamedata
python b1_client_dev.py
if %errorlevel% neq 0 pause
python b2_shard_data.py
if %errorlevel% neq 0 pause
python b3_shard_dev.py
if %errorlevel% neq 0 pause
echo(
set PATH=%RC_PYTHON3_DIR%;%RC_ORIG_PATH%
cd /d %RC_ROOT%\code\tool\quick_start
python print_summary.py
if %errorlevel% neq 0 pause
cd /d %RC_ROOT%
echo(
title Ryzom Core Configuration: Ready
echo To rebuild the client, server and tools from source with the current configuration, run the `code_configure_rebuild_all` script.
echo(
echo Ready
pause
