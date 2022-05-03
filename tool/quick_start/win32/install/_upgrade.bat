@echo off
rem Do not modify this file.
rem Things will break.
title Ryzom Core Upgrade
call %~dp0.nel\path_config.bat
rem Only modify the following file.
call %RC_CODE_DIR%\tool\quick_start\win32\upgrade.bat
cd /d %RC_CODE_DIR%
git pull
if %errorlevel% neq 0 pause
cd /d %RC_ROOT%
call _configure.bat
if %errorlevel% neq 0 pause
