@echo off
echo Installing translation file into ryzom...

xcopy /Y translated\*.uxt ..\client\data\gamedev\language\
REM Copy translated files in client directory...
xcopy /Y translated\*.uxt ..\..\client\data\gamedev\language\
xcopy /Y translated\skill_*.txt ..\..\client\data\gamedev\language\
REM xcopy /Y translated\brick_*.txt ..\client\data\gamedev\language\
xcopy /Y translated\item_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\creature_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\sbrick_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\sphrase_*.txt ..\..\client\data\gamedev\language\
REM xcopy /Y translated\tribe_*.txt ..\client\data\gamedev\language\
xcopy /Y translated\place_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\faction_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\title_*.txt ..\..\client\data\gamedev\language\
xcopy /Y translated\outpost_*.txt ..\..\client\data\gamedev\language\

	
xcopy /Y translated\*.txt ..\server\data_shard\language\

echo Done.
pause