call ..\.nel\path_config.bat
if not defined RC_SIGNTOOL_SHA1 echo NOTE: SignTool Sha1 is not set in .nel\config.json, the files are not signed
rmdir /s /q ryzom_server_win_x64
if %errorlevel% neq 0 pause
mkdir ryzom_server_win_x64
cd ryzom_server_win_x64

xcopy %RC_ROOT%\build_win32\server_dev\bin\Release\*.exe .\
del crash_report.exe

if defined RC_SIGNTOOL_SHA1 "%RC_SIGNTOOL_EXE%" sign /sha1 %RC_SIGNTOOL_SHA1% /v /fd sha256 /tr %RC_SIGNTOOL_TIMESTAMP% /td sha256 "*.exe"
if defined RC_SIGNTOOL_SHA1 "%RC_SIGNTOOL_EXE%" sign /sha1 %RC_SIGNTOOL_SHA1% /v /fd sha256 /tr %RC_SIGNTOOL_TIMESTAMP% /td sha256 "*.dll"

copy "C:\2026q4_external_v143_x64\libjpeg\bin\jpeg62.dll" "jpeg62.dll"
copy "C:\2026q4_external_v143_x64\giflib\bin\gif.dll" "gif.dll"
copy "C:\2026q4_external_v143_x64\libpng\bin\libpng16.dll" "libpng16.dll"
copy "C:\2026q4_external_v143_x64\zlib\bin\libz.dll" "libz.dll"
copy "C:\2026q4_external_v143_x64\openal-soft\bin\OpenAL32.dll" "OpenAL32.dll"
copy "C:\2026q4_external_v143_x64\openssl\bin\libcrypto-3-x64.dll" "libcrypto-3-x64.dll"
copy "C:\2026q4_external_v143_x64\openssl\bin\libssl-3-x64.dll" "libssl-3-x64.dll"
rem copy "C:\2026q4_external_v143_x64\curl\bin\libcurl.dll" "libcurl.dll"
copy "C:\2026q4_external_v143_x64\freetype\bin\freetype.dll" "freetype.dll"
copy "C:\2026q4_external_v143_x64\libxml2\bin\libxml2.dll" "libxml2.dll"
copy "C:\2026q4_external_v143_x64\lua\bin\lua.dll" "lua.dll"
copy "C:\2026q4_external_v143_x64\ogg\bin\ogg.dll" "ogg.dll"
copy "C:\2026q4_external_v143_x64\vorbis\bin\vorbis.dll" "vorbis.dll"
copy "C:\2026q4_external_v143_x64\vorbis\bin\vorbisfile.dll" "vorbisfile.dll"
copy "C:\2026q4_external_v143_x64\mariadb-connector-c\bin\libmariadb.dll" "libmariadb.dll"

cd ..

exit
