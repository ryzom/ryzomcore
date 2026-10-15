goto :checktools
:nogit
echo ERROR: Git is not installed
pause
exit
:checktools
where /q git
if %errorlevel% neq 0 goto :nogit

cd /d %RC_ROOT%\graphics
git pull
if %errorlevel% neq 0 pause

cd /d %RC_ROOT%\leveldesign
git pull
if %errorlevel% neq 0 pause

cd /d %RC_ROOT%\patchman
git pull
if %errorlevel% neq 0 pause

cd /d %RC_ROOT%\sound
git pull
if %errorlevel% neq 0 pause

rem Removing this file causes configure.bat to run the build pipeline
del %RC_ROOT%\pipeline\install\data_leveldesign\sheet_id.bin
set RC_UPGRADE_CONFIGURE=BUILD
