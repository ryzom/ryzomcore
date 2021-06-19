call path_config.bat
cd /d %RC_ROOT%
set PATH=%RC_ROOT%\build\dev_x64\bin\Release;C:\2019q4_external_v142_x64\zlib\bin;C:\2019q4_external_v142_x64\curl\bin;C:\2019q4_external_v142_x64\openssl\bin;C:\2019q4_external_v142_x64\libjpeg\bin;C:\2019q4_external_v142_x64\libpng\bin;C:\2019q4_external_v142_x64\libiconv\bin;C:\2019q4_external_v142_x64\libxml2\bin;C:\2019q4_external_v142_x64\freetype\bin;C:\2019q4_external_v142_x64\squish\bin;C:\2019q4_external_v142_x64\ogg\bin;C:\2019q4_external_v142_x64\vorbis\bin;C:\2019q4_external_v142_x64\openal-soft\bin;C:\2019q4_external_v142_x64\lua\bin;C:\2019q4_external_v142_x64\luabind\bin;C:\2019q4_external_v142_x64\mariadb-connector-c\bin;C:\2019q4_external_v142_x64\assimp\bin;C:\2019q4_external_v142_x64\qt5\bin;C:\2019q4_external_v142_x64\qt5\plugins;%PATH%
cd pipeline\client_dev_live
start ryzom_client_dev_r.exe
