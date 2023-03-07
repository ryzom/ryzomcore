@echo off

if "x%ROOTPATH%x" == "xx" (
	echo upgd_nl.bat can only be launched from updt_nl.bat
	goto end
)

rem We used this hack because to client 2.1 to 3.0 patch,
rem VC++ 2013 DLLs are using the same name and are deleted during patch
if exist %ROOTPATH%\msvcp120_win32.dll (
	if exist %ROOTPATH%\msvcp120.dll del %ROOTPATH%\msvcp120.dll
	copy /Y %ROOTPATH%\msvcp120_win32.dll %ROOTPATH%\msvcp120.dll
)

if exist %ROOTPATH%\msvcr120_win32.dll (
	if exist %ROOTPATH%\msvcr120.dll del %ROOTPATH%\msvcr120.dll
	copy /Y %ROOTPATH%\msvcr120_win32.dll %ROOTPATH%\msvcr120.dll
)

if exist %ROOTPATH%\msvcp120_win64.dll (
	if exist %ROOTPATH%\msvcp120.dll del %ROOTPATH%\msvcp120.dll
	copy /Y %ROOTPATH%\msvcp120_win64.dll %ROOTPATH%\msvcp120.dll
)

if exist %ROOTPATH%\msvcr120_win64.dll (
	if exist %ROOTPATH%\msvcr120.dll del %ROOTPATH%\msvcr120.dll
	copy /Y %ROOTPATH%\msvcr120_win64.dll %ROOTPATH%\msvcr120.dll
)

if exist %ROOTPATH%\..\ryzom_installer_qt_r.exe (
	if exist %ROOTPATH%\..\ryzom_installer_qt_r.exe del %ROOTPATH%\..\ryzom_installer_qt_r.exe
	copy /Y %ROOTPATH%\ryzom_installer_qt_r.exe %ROOTPATH%\..
	if exist %ROOTPATH%\..\msvcp120.dll del %ROOTPATH%\..\msvcp120.dll
	copy /Y %ROOTPATH%\msvcp120.dll %ROOTPATH%\..
	if exist %ROOTPATH%\..\msvcr120.dll del %ROOTPATH%\..\msvcr120.dll
	copy /Y %ROOTPATH%\msvcr120.dll %ROOTPATH%\..
)

:end
