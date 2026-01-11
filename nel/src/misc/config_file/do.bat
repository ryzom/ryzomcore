@echo off
echo Generating cf_lexical.cpp
flex -f -8 -Pcf -Scf_flex.skl -ocf_lexical.cpp cf_lexical.lpp

echo Generating cf_gramatical.cpp
set BISON_SIMPLE=cf_bison.simple
bison -d -p cf -o cf_gramatical.cpp cf_gramatical.ypp
move cf_gramatical.hpp cf_gramatical.h
pause
