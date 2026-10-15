if not exist R:\_r_init.bat call %~dp0_r_init.bat
if /I "%RC_ROOT%"=="" call %~dp0.nel\path_config.bat
goto :check
:baddir
echo ERROR: A different environment has already been mounted to R:
echo(
pause
exit
:check
fc /b R:\.nel\path_config.bat %RC_ROOT%\.nel\path_config.bat > nul
if errorlevel 1 goto baddir
