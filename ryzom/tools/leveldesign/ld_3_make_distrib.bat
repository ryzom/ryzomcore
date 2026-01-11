REM ************ Upload LevelDesign Tools


mkdir \\server\code\tools
rmdir /s /q \\server\code\tools\leveldesign
mkdir \\server\code\tools\leveldesign
mkdir \\server\code\tools\leveldesign\zoneBitmaps
mkdir \\server\code\tools\leveldesign\zoneLigos
mkdir \\server\code\tools\leveldesign\zones
rem mkdir \\server\code\tools\leveldesign\common
rem mkdir \\server\code\tools\leveldesign\common\dfn

REM --- Upload update batch

xcopy "r:\code_private\nel\tools\leveldesign\update_tools.bat" "\\server\code\tools\leveldesign" /D

REM --- Upload Binaries

REM - WorldEditor

xcopy "r:\code_private\nel\tools\3d\ligo\WorldEditor_Exe\ReleaseDebug\WorldEditor_exe.exe" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\3d\ligo\WorldEditor\ReleaseDebug\WorldEditor_rd.dll" "\\server\code\tools\leveldesign" /D
xcopy "r:\code\nel\lib\nel_drv_opengl_win_rd.dll" "\\server\code\tools\leveldesign" /D
xcopy "r:\code\tool\zviewer\ReleaseDebug\zviewer.exe" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\3d\ligo\WorldEditor\readme.txt" "\\server\code\tools\leveldesign" /D
rename "\\server\code\tools\leveldesign\readme.txt" "worldeditor.txt"

REM - Georges

xcopy "r:\code_private\nel\tools\leveldesign\georges_dll\georges_dll_rd.dll" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\leveldesign\georges_exe\georges_exe.exe" "\\server\code\tools\leveldesign" /D

REM - LogicEditor

xcopy "r:\code_private\nel\tools\leveldesign\logic_editor_exe\logic_editor_rd.dll" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\leveldesign\logic_editor_exe\releasedebug\logic_editor_exe.exe" "\\server\code\tools\leveldesign" /D

REM - Master

xcopy "r:\code_private\nel\tools\leveldesign\master\releasedebug\master.exe" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\leveldesign\master\n019003l.pfb" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\leveldesign\master\readme.txt" "\\server\code\tools\leveldesign" /D
rename "\\server\code\tools\leveldesign\readme.txt" "master.txt"
rem xcopy "r:\code_private\nel\tools\leveldesign\test_root\common\dfn\*.*" "\\server\code\tools\leveldesign\common\dfn" /D
xcopy "r:\code_private\nel\tools\leveldesign\test_root\ligoscape.cfg" "\\server\code\tools\leveldesign" /D
xcopy "r:\code_private\nel\tools\leveldesign\test_root\zoneBitmaps\_unused_.tga" "\\server\code\tools\leveldesign\zoneBitmaps" /D

REM --- Backup the build

xcopy "\\server\code\tools\leveldesign\*.*" "\\server\code\tools\leveldesign backup\" /E
ren_date "\\server\code\tools\leveldesign backup" "\\server\code\tools\leveldesign"

pause
cd \code\nel\tools\leveldesign