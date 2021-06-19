@echo off
rem title Ryzom Core Web
rem call web_off
cd.>.\nginx\logs\access.log
cd.>.\nginx\logs\error.log
call ..\path_config.bat
mkdir ..\save
mkdir ..\save\dev
mkdir ..\save\dev\mysql
mkdir ..\save\dev\www
set PATH=%~dp0\php;%~dp0\mariadb;%~dp0\nginx;%PATH%
call mariadb_setup.bat
call nginx_setup.bat
start ..\distribution\ryzom_tools_win_x64\servdash.exe web_dash.ini
