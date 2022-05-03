@echo off
rem title Ryzom Core Web
rem call web_off
cd.>.\nginx\logs\access.log
cd.>.\nginx\logs\error.log
call ..\.nel\path_config.bat
mkdir ..\save
mkdir ..\save\dev
mkdir ..\save\dev\mysql
mkdir ..\save\dev\www
set PATH=%~dp0\php;%~dp0\mariadb;%~dp0\nginx;%PATH%
call mariadb_setup.bat
call nginx_setup.bat
start ..\external\utils\servdash web_dash.ini
