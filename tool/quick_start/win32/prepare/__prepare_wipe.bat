
echo WIPE
echo WIPE
echo WIPE
pause

cd external
cmd /C "call web_wipe"
cd ..

del external\*.*
del redist\*.bat

rmdir /S /Q .\pipeline\client
rmdir /S /Q .\pipeline\shard_dev
rmdir /S /Q .\pipeline\export\shard
rmdir /S /Q .\pipeline\export\common\data_common
rmdir /S /Q .\pipeline\export\common\exedll
rmdir /S /Q .\pipeline\export\common\exedll_win32
rmdir /S /Q .\pipeline\export\common\exedll_win64
rmdir /S /Q .\pipeline\export\common\exedll_linux32
rmdir /S /Q .\pipeline\export\common\exedll_linux64
rmdir /S /Q .\pipeline\export\common\leveldesign
rmdir /S /Q .\pipeline\export\common\sound

rmdir /S /Q .\build_docker
rmdir /S /Q .\build_win32\plugin_max
rmdir /S /Q .\build_win32\samples

rmdir /S /Q .\build_win32\client_dev\.vs
rmdir /S /Q .\build_win32\client_dev\CMakeFiles
rmdir /S /Q .\build_win32\client_dev\lib
rmdir /S /Q .\build_win32\client_dev\nel
rmdir /S /Q .\build_win32\client_dev\ryzom
rmdir /S /Q .\build_win32\client_dev\x64
rmdir /S /Q .\build_win32\client_dev\bin\Debug
del build_win32\client_dev\*.*

rmdir /S /Q .\build_win32\server_dev\.vs
rmdir /S /Q .\build_win32\server_dev\CMakeFiles
rmdir /S /Q .\build_win32\server_dev\lib
rmdir /S /Q .\build_win32\server_dev\nel
rmdir /S /Q .\build_win32\server_dev\ryzom
rmdir /S /Q .\build_win32\server_dev\x64
rmdir /S /Q .\build_win32\server_dev\bin\Debug
del build_win32\server_dev\*.*

rmdir /S /Q .\build_win32\tools\.vs
rmdir /S /Q .\build_win32\tools\CMakeFiles
rmdir /S /Q .\build_win32\tools\lib
rmdir /S /Q .\build_win32\tools\nel
rmdir /S /Q .\build_win32\tools\ryzom
rmdir /S /Q .\build_win32\tools\x64
rmdir /S /Q .\build_win32\tools\bin\Debug
del build_win32\tools\*.*

del build_win32\*.*

rmdir /S /Q .\.nel
del *.*

echo Ready
pause
