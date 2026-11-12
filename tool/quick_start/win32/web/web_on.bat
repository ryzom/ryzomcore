@echo off
rem Starts the development web stack of this root in the service dashboard: MariaDB, PHP and nginx of the externals
rem package, with the configuration configure_web.py wrote in RC_WEB_DIR (pipeline\shard_dev\web)
rem title Ryzom Core Web
if exist "%RC_WEB_DIR%\web_dash.ini" goto :configured
echo ERROR: "%RC_WEB_DIR%\web_dash.ini" was not found, run _configure.bat to configure the web
pause
exit /b 1
:configured
cd.>"%RC_WEB_DIR%\nginx\logs\access.log"
cd.>"%RC_WEB_DIR%\nginx\logs\error.log"
mkdir "%RC_ROOT%\save" > nul 2> nul
mkdir "%RC_ROOT%\save\dev" > nul 2> nul
mkdir "%RC_ROOT%\save\dev\mysql" > nul 2> nul
mkdir "%RC_ROOT%\save\dev\www" > nul 2> nul
call "%~dp0mariadb_setup.bat"
call "%RC_ROOT%\_servdash.bat" "%RC_WEB_DIR%\web_dash.ini"
