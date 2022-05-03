call ..\.nel\path_config.bat
rmdir /s /q ryzom_server_win_x64
if %errorlevel% neq 0 pause
mkdir ryzom_server_win_x64
cd ryzom_server_win_x64

xcopy %RC_ROOT%\build_win32\server_dev\bin\Release\*.exe .\
del crash_report.exe

"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 f0ef3c22d0373d2d6e3735207194d097c8f69877 /v /t http://timestamp.comodoca.com/authenticode "*.exe"
"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Bin\signtool.exe" sign /sha1 f0ef3c22d0373d2d6e3735207194d097c8f69877 /v /t http://timestamp.comodoca.com/authenticode "*.dll"

copy "C:\2022q2_external_v143_x64\libjpeg\bin\jpeg62.dll" "jpeg62.dll"
copy "C:\2022q2_external_v143_x64\libpng\bin\libpng16.dll" "libpng16.dll"
copy "C:\2022q2_external_v143_x64\zlib\bin\zlib.dll" "zlib.dll"
copy "C:\2022q2_external_v143_x64\openal-soft\bin\OpenAL32.dll" "OpenAL32.dll"
rem copy "C:\2022q2_external_v143_x64\openssl\bin\libcrypto-1_1-x64.dll" "libcrypto-1_1-x64.dll"
rem copy "C:\2022q2_external_v143_x64\openssl\bin\libssl-1_1-x64.dll" "libssl-1_1-x64.dll"
rem copy "C:\2022q2_external_v143_x64\curl\bin\libcurl.dll" "libcurl.dll"
copy "C:\2022q2_external_v143_x64\freetype\bin\freetype.dll" "freetype.dll"
copy "C:\2022q2_external_v143_x64\libxml2\bin\libxml2.dll" "libxml2.dll"
copy "C:\2022q2_external_v143_x64\lua\bin\lua.dll" "lua.dll"
copy "C:\2022q2_external_v143_x64\ogg\bin\ogg.dll" "ogg.dll"
copy "C:\2022q2_external_v143_x64\vorbis\bin\vorbis.dll" "vorbis.dll"
copy "C:\2022q2_external_v143_x64\vorbis\bin\vorbisfile.dll" "vorbisfile.dll"
copy "C:\2022q2_external_v143_x64\mariadb-connector-c\bin\libmariadb.dll" "libmariadb.dll"

cd ..

exit
