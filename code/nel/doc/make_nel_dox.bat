@echo off

del html\nel\*.* /Q
set WORKDIR=%CD%

cd ..
set CURDIR=%CD%
cd %WORKDIR%
doxygen nel.dox -DCURDIR

pause
