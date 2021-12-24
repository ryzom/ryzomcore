if not exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" goto :no_build
call ..\path_config.bat
C:
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
cd /d %RC_ROOT%\build
call patch_version.bat
if %errorlevel% neq 0 pause
cd 3dsmax
rem goto :skipmax
if not exist .\2017_x64\CMakeCache.txt goto :no_2017
cd 2017_x64
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_2017
if not exist .\2018_x64\CMakeCache.txt goto :no_2018
cd 2018_x64
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_2018
if not exist .\2019_x64\CMakeCache.txt goto :no_2019
cd 2019_x64
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_2019
if not exist .\2020_x64\CMakeCache.txt goto :no_2020
cd 2020_x64
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_2020
if not exist .\2022_x64\CMakeCache.txt goto :no_2022
cd 2022_x64
cmake -DNL_VERSION_PATCH=%CLIENT_PATCH_VERSION% .
if %errorlevel% neq 0 pause
rem FIXME msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
if %errorlevel% neq 0 pause
cd ..
:no_2022
rem :skipmax
cd ..
:no_build
