@echo off
echo Installing translation file into ryzom...

REM Copy translated files in client directory...
xcopy /Y translated\*.uxt ..\..\client\data\gamedev\language\
xcopy /Y translated\skill_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\item_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\creature_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\sbrick_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\sphrase_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\place_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\faction_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\title_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\outpost_*.txt ..\..\client\data\gamedev\language\

echo Done.
