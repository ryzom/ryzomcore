if not exist .\fv_x86\CMakeCache.txt goto :no_build
call ..\path_config.bat
C:
cd C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
cd /d %RC_ROOT%\build
call patch_version.bat
if %errorlevel% neq 0 pause
cd fv_x86
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_build
