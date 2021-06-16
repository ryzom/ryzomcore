if not exist .\samples_x64\CMakeCache.txt goto :no_build
call ..\path_config.bat
C:
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
cd /d %RC_ROOT%\build
call patch_version.bat
if %errorlevel% neq 0 pause
cd samples_x64
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_build
