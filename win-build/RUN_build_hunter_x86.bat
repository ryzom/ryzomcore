call ..\path_config.bat
cd /d C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
cd /d "%RC_ROOT%\win-build"
mkdir fv_x86
cd fv_x86
cmake -G "NMake Makefiles JOM" -S ..\.. -DHUNTER_ENABLED=ON -DHUNTER_CONFIGURATION_TYPES=Release -DWITH_NEL_TESTS=OFF -DWITH_NEL_SAMPLES=OFF -DWITH_NEL_TOOLS=OFF -DWITH_RYZOM_TOOLS=OFF -DWITH_RYZOM_SERVER=OFF -DWITH_RYZOM_CLIENT=ON
pause