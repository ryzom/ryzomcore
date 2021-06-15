if not exist R:\_r_init.bat call _r_init.bat
if /I "%RC_ROOT%"=="" call path_config.bat
goto :check
:baddir
echo ERROR: A different environment has already been mounted to R:
echo(
pause
exit
:check
fc /b R:\georges.lnk %RC_ROOT%\georges.lnk > nul
if errorlevel 1 goto baddir
