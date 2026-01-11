@echo off

del html\nelns\*.* /Q
set WORKDIR=%CD%

cd ..\..\nelns
set CURDIR=%CD%
cd %WORKDIR%
doxygen nelns.dox -DCURDIR

pause
