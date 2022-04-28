@echo off
title Ryzom Core Docker Configuration
call %~dp0.nel\path_config.bat
set PATH=%RC_PATH%;%RC_PYTHON3_DIR%;%PATH%
cd /d %RC_CODE_DIR%\tool\quick_start
python configure_docker.py
if %errorlevel% neq 0 pause
python configure_toolchains.py
if %errorlevel% neq 0 pause
python configure_targets.py
if %errorlevel% neq 0 pause
python print_summary.py
if %errorlevel% neq 0 pause
echo(
title Ryzom Core Docker Configuration: Ready
echo Ready
pause
