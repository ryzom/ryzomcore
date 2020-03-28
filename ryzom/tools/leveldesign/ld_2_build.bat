r:

cd \code_private\nel\tools\leveldesign

rem set visual path

call "C:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

rem compiles misc library in all config

msdev leveldesign.dsw /MAKE "master - Win32 ReleaseDebug"

rem wait the user input to quit

pause
cd \code\nel\tools\leveldesign