@echo off
title Ryzom Core Web
call web_off
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
cd.>.\nginx\logs\access.log
cd.>.\nginx\logs\error.log
call ..\path_config.bat
mkdir ..\save
mkdir ..\save\dev
mkdir ..\save\dev\mysql
mkdir ..\save\dev\www
set PATH=%~dp0\php;%~dp0\mariadb;%~dp0\nginx;%PATH%
wt -w "Ryzom Core Web" -d . -p "Command Prompt" access-log.bat
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
wt -w "Ryzom Core Web" -d . -p "Command Prompt" error-log.bat
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
wt -w "Ryzom Core Web" -d . -p "Command Prompt" mariadb.bat
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
wt -w "Ryzom Core Web" -d . -p "Command Prompt" php.bat
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
wt -w "Ryzom Core Web" -d . -p "Command Prompt" nginx.bat
