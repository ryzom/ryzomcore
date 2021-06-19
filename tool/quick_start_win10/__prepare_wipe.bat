cd external
cmd /C "call web_wipe"
cd ..
rmdir /S /Q .\pipeline\shard_dev
rmdir /S /Q .\pipeline\export\shard
rmdir /S /Q .\pipeline\export\common\data_common
rmdir /S /Q .\pipeline\export\common\exedll
rmdir /S /Q .\pipeline\export\common\leveldesign
rmdir /S /Q .\pipeline\export\common\sound
rmdir /S /Q .\build\3dsmax
rmdir /S /Q .\build\samples_x64
rmdir /S /Q .\build\tools_x64
rmdir /S /Q .\build\dev_x64\CMakeFiles
rmdir /S /Q .\build\dev_x64\lib
rmdir /S /Q .\build\dev_x64\nel
rmdir /S /Q .\build\dev_x64\ryzom
rmdir /S /Q .\build\dev_x64\x64
del build\dev_x64\*.*
rmdir /S /Q .\build\fv_x64\CMakeFiles
rmdir /S /Q .\build\fv_x64\lib
rmdir /S /Q .\build\fv_x64\nel
rmdir /S /Q .\build\fv_x64\ryzom
rmdir /S /Q .\build\fv_x64\x64
del build\fv_x64\*.*
rmdir /S /Q .\build\fv_x86\CMakeFiles
rmdir /S /Q .\build\fv_x86\lib
rmdir /S /Q .\build\fv_x86\nel
rmdir /S /Q .\build\fv_x86\ryzom
rmdir /S /Q .\build\fv_x86\ALL_BUILD.dir
del build\fv_x86\*.*
rmdir /S /Q .\build\server_gcc\CMakeFiles
rmdir /S /Q .\build\server_gcc\lib
rmdir /S /Q .\build\server_gcc\nel
rmdir /S /Q .\build\server_gcc\ryzom
del build\server_gcc\*.*
rmdir /S /Q .\build\server_x64\CMakeFiles
rmdir /S /Q .\build\server_x64\lib
rmdir /S /Q .\build\server_x64\nel
rmdir /S /Q .\build\server_x64\ryzom
rmdir /S /Q .\build\server_x64\x64
del build\server_x64\*.*
del .\path_config.bat
del *.lnk
