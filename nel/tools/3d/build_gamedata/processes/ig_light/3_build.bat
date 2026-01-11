@echo off

REM Log error
echo ------- > log.log
echo --- Ig lighting >> log.log
echo ------- >> log.log
echo ------- 
echo --- Ig lighting
echo ------- 
date /T >> log.log
date /T

REM light All the ..\ig\ig_other directory

ig_lighter.exe ..\ig\ig_other ig_other_lighted ..\..\cfg\properties.cfg

