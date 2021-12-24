call ..\path_config.bat
rmdir /s /q external_x64
if %errorlevel% neq 0 pause
mkdir external_x64
cd external_x64
copy "C:\2019q4_external_v142_x64\zlib\bin\zlib.dll" zlib.dll
copy "C:\2019q4_external_v142_x64\curl\bin\libcurl.dll" libcurl.dll
copy "C:\2019q4_external_v142_x64\curl\bin\curl.exe" curl.exe
copy "C:\2019q4_external_v142_x64\freetype\bin\freetype.dll" freetype.dll
copy "C:\2019q4_external_v142_x64\lua\bin\lua.dll" lua.dll
copy "C:\2019q4_external_v142_x64\lua\bin\lua.exe" lua.exe
copy "C:\2019q4_external_v142_x64\luabind\bin\luabind.dll" luabind.dll
copy "C:\2019q4_external_v142_x64\assimp\bin\assimp.dll" assimp.dll
copy "C:\2019q4_external_v142_x64\assimp\bin\assimp.exe" assimp.exe
copy "C:\2019q4_external_v142_x64\assimp\bin\assimp_viewer.exe" assimp_viewer.exe
copy "C:\2019q4_external_v142_x64\libiconv\bin\libcharset.dll" libcharset.dll
copy "C:\2019q4_external_v142_x64\libiconv\bin\libiconv.dll" libiconv.dll
copy "C:\2019q4_external_v142_x64\libjpeg\bin\jpeg62.dll" jpeg62.dll
copy "C:\2019q4_external_v142_x64\libjpeg\bin\turbojpeg.dll" turbojpeg.dll
copy "C:\2019q4_external_v142_x64\libpng\bin\libpng16.dll" libpng16.dll
copy "C:\2019q4_external_v142_x64\libxml2\bin\libxml2.dll" libxml2.dll
copy "C:\2019q4_external_v142_x64\ogg\bin\ogg.dll" ogg.dll
copy "C:\2019q4_external_v142_x64\vorbis\bin\vorbis.dll" vorbis.dll
copy "C:\2019q4_external_v142_x64\vorbis\bin\vorbisfile.dll" vorbisfile.dll
copy "C:\2019q4_external_v142_x64\openssl\bin\libeay32.dll" libeay32.dll
copy "C:\2019q4_external_v142_x64\openssl\bin\ssleay32.dll" ssleay32.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5Core.dll" Qt5Core.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5Gui.dll" Qt5Gui.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5Network.dll" Qt5Network.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5PrintSupport.dll" Qt5PrintSupport.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5Test.dll" Qt5Test.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5Widgets.dll" Qt5Widgets.dll
copy "C:\2019q4_external_v142_x64\qt5\bin\Qt5Xml.dll" Qt5Xml.dll
mkdir imageformats
cd imageformats
copy "C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qgif.dll" qgif.dll
copy "C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qjpeg.dll" qjpeg.dll
copy "C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qtga.dll" qtga.dll
copy "C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qtiff.dll" qtiff.dll
copy "C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qwbmp.dll" qwbmp.dll
copy "C:\2019q4_external_v142_x64\qt5\plugins\imageformats\qwebp.dll" qwebp.dll
cd ..
mkdir platforms
cd platforms
copy "C:\2019q4_external_v142_x64\qt5\plugins\platforms\qwindows.dll" qwindows.dll
cd ..
mkdir styles
cd styles
copy "C:\2019q4_external_v142_x64\qt5\plugins\styles\qwindowsvistastyle.dll" qwindowsvistastyle.dll
cd ..
cd ..

rmdir /s /q external_x86
if %errorlevel% neq 0 pause
mkdir external_x86
cd external_x86
copy "C:\2019q4_external_v90_x86\zlib\bin\zlib.dll" zlib.dll
copy "C:\2019q4_external_v90_x86\curl\bin\libcurl.dll" libcurl.dll
copy "C:\2019q4_external_v90_x86\curl\bin\curl.exe" curl.exe
copy "C:\2019q4_external_v90_x86\freetype\bin\freetype.dll" freetype.dll
copy "C:\2019q4_external_v90_x86\lua\bin\lua.dll" lua.dll
copy "C:\2019q4_external_v90_x86\lua\bin\lua.exe" lua.exe
copy "C:\2019q4_external_v90_x86\luabind\bin\luabind.dll" luabind.dll
copy "C:\2019q4_external_v90_x86\libiconv\bin\libcharset.dll" libcharset.dll
copy "C:\2019q4_external_v90_x86\libiconv\bin\libiconv.dll" libiconv.dll
copy "C:\2019q4_external_v90_x86\libjpeg\bin\jpeg62.dll" jpeg62.dll
copy "C:\2019q4_external_v90_x86\libjpeg\bin\turbojpeg.dll" turbojpeg.dll
copy "C:\2019q4_external_v90_x86\libpng\bin\libpng16.dll" libpng16.dll
copy "C:\2019q4_external_v90_x86\libxml2\bin\libxml2.dll" libxml2.dll
copy "C:\2019q4_external_v90_x86\ogg\bin\ogg.dll" ogg.dll
copy "C:\2019q4_external_v90_x86\vorbis\bin\vorbis.dll" vorbis.dll
copy "C:\2019q4_external_v90_x86\vorbis\bin\vorbisfile.dll" vorbisfile.dll
copy "C:\2019q4_external_v90_x86\openssl\bin\libeay32.dll" libeay32.dll
copy "C:\2019q4_external_v90_x86\openssl\bin\ssleay32.dll" ssleay32.dll
cd ..

exit

