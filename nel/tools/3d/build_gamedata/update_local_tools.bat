@echo off

SET COPYCMD=/Y

REM -- Backup config files
copy cfg\*.cfg cfg\*.bak

REM -- Copy tools
xcopy r:\code\nel\tools\3d\build_gamedata\*.bat . /E /D
xcopy r:\code\nel\tools\3d\build_gamedata\*.sh . /E /D
xcopy r:\code\nel\tools\3d\build_gamedata\*.cfg . /E /D
xcopy r:\code\nel\tools\3d\build_gamedata\*.ms . /E /D
xcopy r:\code\nel\tools\3d\build_gamedata\*.txt . /E /D

REM -- Restore config files
copy cfg\*.bak cfg\*.cfg

REM -- Delete backuped config files
del cfg\*.bak
