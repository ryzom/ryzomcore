call ..\.nel\path_config.bat
if not defined RC_SIGNTOOL_SHA1 echo NOTE: SignTool Sha1 is not set in .nel\config.json, the files are not signed
rmdir /s /q ryzom_client_win_x64
if %errorlevel% neq 0 pause
mkdir ryzom_client_win_x64
cd ryzom_client_win_x64

copy %RC_ROOT%\build_win32\client_dev\bin\Release\ryzom_client_dev_r.exe ryzom_client_dev_r.exe

copy %RC_ROOT%\build_win32\client_dev\bin\Release\nel_drv_direct3d_win_r.dll nel_drv_direct3d_win_r.dll
copy %RC_ROOT%\build_win32\client_dev\bin\Release\nel_drv_opengl_win_r.dll nel_drv_opengl_win_r.dll
copy %RC_ROOT%\build_win32\client_dev\bin\Release\nel_drv_openal_win_r.dll nel_drv_openal_win_r.dll
copy %RC_ROOT%\build_win32\client_dev\bin\Release\nel_drv_fmod_win_r.dll nel_drv_fmod_win_r.dll
copy %RC_ROOT%\build_win32\client_dev\bin\Release\nel_drv_xaudio2_win_r.dll nel_drv_xaudio2_win_r.dll
copy %RC_ROOT%\build_win32\client_dev\bin\Release\nel_drv_dsound_win_r.dll nel_drv_dsound_win_r.dll

copy %RC_ROOT%\build_win32\client_dev\bin\Release\ryzom_client_patcher.exe ryzom_client_patcher.exe
copy %RC_ROOT%\build_win32\client_dev\bin\Release\ryzom_configuration_qt_r.exe ryzom_configuration_qt_r.exe
copy %RC_ROOT%\build_win32\client_dev\bin\Release\crash_report.exe crash_report.exe

if defined RC_SIGNTOOL_SHA1 "%RC_SIGNTOOL_EXE%" sign /sha1 %RC_SIGNTOOL_SHA1% /v /fd sha256 /tr %RC_SIGNTOOL_TIMESTAMP% /td sha256 "*.exe"
if defined RC_SIGNTOOL_SHA1 "%RC_SIGNTOOL_EXE%" sign /sha1 %RC_SIGNTOOL_SHA1% /v /fd sha256 /tr %RC_SIGNTOOL_TIMESTAMP% /td sha256 "*.dll"

copy "C:\2026q4_external_v143_x64\libjpeg\bin\jpeg62.dll" "jpeg62.dll"
copy "C:\2026q4_external_v143_x64\giflib\bin\gif.dll" "gif.dll"
copy "C:\2026q4_external_v143_x64\libpng\bin\libpng16.dll" "libpng16.dll"
copy "C:\2026q4_external_v143_x64\zlib\bin\libz.dll" "libz.dll"
copy "C:\2026q4_external_v143_x64\openal-soft\bin\OpenAL32.dll" "OpenAL32.dll"
copy "C:\2026q4_external_v143_x64\openssl\bin\libcrypto-3-x64.dll" "libcrypto-3-x64.dll"
copy "C:\2026q4_external_v143_x64\openssl\bin\libssl-3-x64.dll" "libssl-3-x64.dll"
copy "C:\2026q4_external_v143_x64\curl\bin\libcurl.dll" "libcurl.dll"
copy "C:\2026q4_external_v143_x64\freetype\bin\freetype.dll" "freetype.dll"
copy "C:\2026q4_external_v143_x64\libxml2\bin\libxml2.dll" "libxml2.dll"
copy "C:\2026q4_external_v143_x64\lua\bin\lua.dll" "lua.dll"
copy "C:\2026q4_external_v143_x64\ogg\bin\ogg.dll" "ogg.dll"
copy "C:\2026q4_external_v143_x64\vorbis\bin\vorbis.dll" "vorbis.dll"
copy "C:\2026q4_external_v143_x64\vorbis\bin\vorbisfile.dll" "vorbisfile.dll"

copy C:\2026q4_external_v143_x64\qt5\bin\Qt5Core.dll Qt5Core.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5Gui.dll Qt5Gui.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5Test.dll Qt5Test.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5Xml.dll Qt5Xml.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5Widgets.dll Qt5Widgets.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5Network.dll Qt5Network.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5PrintSupport.dll Qt5PrintSupport.dll
copy C:\2026q4_external_v143_x64\qt5\bin\Qt5OpenGL.dll Qt5OpenGL.dll
copy C:\2026q4_external_v143_x64\qt5\bin\libGLESv2.dll libGLESv2.dll
copy C:\2026q4_external_v143_x64\qt5\bin\libEGL.dll libEGL.dll

rmdir /s /q platforms
mkdir platforms
cd platforms
copy C:\2026q4_external_v143_x64\qt5\plugins\platforms\qwindows.dll qwindows.dll
cd ..

rmdir /s /q styles
mkdir styles
cd styles
copy C:\2026q4_external_v143_x64\qt5\plugins\styles\qwindowsvistastyle.dll qwindowsvistastyle.dll
cd ..

rmdir /s /q imageformats
mkdir imageformats
cd imageformats
copy C:\2026q4_external_v143_x64\qt5\plugins\imageformats\qjpeg.dll qjpeg.dll
copy C:\2026q4_external_v143_x64\qt5\plugins\imageformats\qgif.dll qgif.dll
copy C:\2026q4_external_v143_x64\qt5\plugins\imageformats\qtga.dll qtga.dll
copy C:\2026q4_external_v143_x64\qt5\plugins\imageformats\qwbmp.dll qwbmp.dll
copy C:\2026q4_external_v143_x64\qt5\plugins\imageformats\qtiff.dll qtiff.dll
copy C:\2026q4_external_v143_x64\qt5\plugins\imageformats\qwebp.dll qwebp.dll
cd ..

cd ..

exit
