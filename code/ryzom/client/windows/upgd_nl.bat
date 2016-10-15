@echo off

if "x%ROOTPATH%x" == "xx" (
	echo upgd_nl.bat can only be launched from updt_nl.bat
	goto end
)

rem We used this hack because to client 2.1 to 3.0 patch,
rem MSVC10 DLLs are using the same name and are deleted during patch
if exist %ROOTPATH%\msvcp100_win32.dll (
	if exist %ROOTPATH%\msvcp100.dll del %ROOTPATH%\msvcp100.dll
	copy /Y %ROOTPATH%\msvcp100_win32.dll %ROOTPATH%\msvcp100.dll
)

if exist %ROOTPATH%\msvcr100_win32.dll (
	if exist %ROOTPATH%\msvcr100.dll del %ROOTPATH%\msvcr100.dll
	copy /Y %ROOTPATH%\msvcr100_win32.dll %ROOTPATH%\msvcr100.dll
)

if exist %ROOTPATH%\msvcp100_win64.dll (
	if exist %ROOTPATH%\msvcp100.dll del %ROOTPATH%\msvcp100.dll
	copy /Y %ROOTPATH%\msvcp100_win64.dll %ROOTPATH%\msvcp100.dll
)

if exist %ROOTPATH%\msvcr100_win64.dll (
	if exist %ROOTPATH%\msvcr100.dll del %ROOTPATH%\msvcr100.dll
	copy /Y %ROOTPATH%\msvcr100_win64.dll %ROOTPATH%\msvcr100.dll
)

if exist %ROOTPATH%\..\ryzom_installer_qt_r.exe (
	if exist %ROOTPATH%\..\ryzom_installer_qt_r.exe del %ROOTPATH%\..\ryzom_installer_qt_r.exe
	copy /Y %ROOTPATH%\ryzom_installer_qt_r.exe %ROOTPATH%\..
	if exist %ROOTPATH%\..\msvcp100.dll del %ROOTPATH%\..\msvcp100.dll
	copy /Y %ROOTPATH%\msvcp100.dll %ROOTPATH%\..
	if exist %ROOTPATH%\..\msvcr100.dll del %ROOTPATH%\..\msvcr100.dll
	copy /Y %ROOTPATH%\msvcr100.dll %ROOTPATH%\..
)

:end
