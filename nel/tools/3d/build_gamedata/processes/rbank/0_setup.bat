@echo off

echo **** > null
mkdir retrievers 2> null
mkdir tesselation 2> null
mkdir smooth 2> null
mkdir smooth\preproc 2> null
mkdir raw 2> null
mkdir raw\preproc 2> null
mkdir tag 2> null
mkdir output 2> null
mkdir cmb 2> null
mkdir bbox 2> null

REM Force setup of the small bank to avoid a warning
cd ..\smallbank
call 0_setup.bat
cd ..\rbank

rm null
