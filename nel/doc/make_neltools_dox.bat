@echo off

del html\tool\*.* /Q
set WORKDIR=%CD%

cd ..\..\tool
set CURDIR=%CD%
cd %WORKDIR%
doxygen neltools.dox -DCURDIR

pause
