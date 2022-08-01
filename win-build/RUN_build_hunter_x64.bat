@Echo off
echo Start building process..
call ..\path_config.bat
cd /d C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd /d "%RC_ROOT%\win-build"
mkdir build_fv_x64
cd build_fv_x64
echo Calling cmake
rem cmake -G "NMake Makefiles JOM" -S %RC_ROOT% -DHUNTER_ENABLED=ON -DHUNTER_CONFIGURATION_TYPES=Release -DWITH_SYMBOLS=ON -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_NEL_TOOLS=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON
cmake -S %RC_ROOT% -DHUNTER_ENABLED=ON -DHUNTER_CONFIGURATION_TYPES=Release -DWITH_NEL_TESTS=OFF -DWITH_SYMBOLS=ON -DWITH_NEL_SAMPLES=OFF -DWITH_NEL_TOOLS=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON
if %errorlevel% neq 0 pause
msbuild RyzomCore.sln /m:2 /p:Configuration=Release %MSBUILDEXTRA%
pause
