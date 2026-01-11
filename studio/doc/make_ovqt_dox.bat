@echo off

del html\ovqt\*.* /Q
set WORKDIR=%CD%

cd ..
set CURDIR=%CD%
cd %WORKDIR%
doxygen ovqt.dox -DCURDIR

pause
