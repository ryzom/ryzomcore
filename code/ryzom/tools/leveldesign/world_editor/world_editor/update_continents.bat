@echo off
SET COPYCMD=/Y

IF EXIST excl.txt	del excl.txt
IF NOT EXIST excl.txt (
	echo \CVS\ > excl.txt
	echo \.svn\ >> excl.txt
	echo .log >> excl.txt
)

REM set next line to 'true' or 'false' next line to update pacs information
set RECUP_PACS=false
set SRC_DATA=F:\computers\amiga\share\Database\Landscape\ligo
set SRC_IA=F:\computers\moulinette\moulinette\ai_build_wmap
set SRC_PACS=\\moulinette\ryzom_data\3d\continents


call :COPYCONTINENT bagne 		primes_racines	bagne
call :COPYCONTINENT fyros 		desert		fyros
call :COPYCONTINENT fyros_island	desert		fyros_island
call :COPYCONTINENT fyros_newbie	desert		fyros_newbie
call :COPYCONTINENT indoors 		jungle		indoors
call :COPYCONTINENT matis 		jungle		matis
call :COPYCONTINENT matis_island	jungle		matis_island
call :COPYCONTINENT nexus 		jungle		nexus
call :COPYCONTINENT route_gouffre 	primes_racines	route_gouffre
call :COPYCONTINENT sources		primes_racines	sources_interdites
call :COPYCONTINENT terre		primes_racines	terre_oubliee
call :COPYCONTINENT tryker		lacustre	tryker
call :COPYCONTINENT tryker_island	lacustre	tryker_island
call :COPYCONTINENT tryker_newbie	lacustre	tryker_newbie
call :COPYCONTINENT zorai		jungle		zorai
call :COPYCONTINENT zorai_island	jungle		zorai_island
call :COPYCONTINENT indoors		jungle		indoors

goto :EOF


:COPYCONTINENT
REM  %1 : continent name
REM  %2 : ecosystem name
REM  %3 : ia continent name :(

echo -------------------------------------------------------------------------------
echo Updating files for continent %1
echo -------------------------------------------------------------------------------

IF NOT EXIST continents	 		mkdir continents
IF NOT EXIST continents\%1 		mkdir continents\%1
IF NOT EXIST continents\%1\collisionmap 	mkdir continents\%1\collisionmap
IF NOT EXIST continents\%1\zonebitmaps 	mkdir continents\%1\zonebitmaps
IF NOT EXIST continents\%1\zoneligos 	mkdir continents\%1\zoneligos
REM VL IF NOT EXIST continents\%1\zones 	mkdir continents\%1\zones
REM VL IF NOT EXIST continents\%1\pacs 		mkdir continents\%1\pacs


echo Updating collision maps files...
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\0*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\1*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\2*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\3*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\4*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\5*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\6*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\7*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\8*.tga 			continents\%1\collisionmap\
xcopy /D /EXCLUDE:excl.txt %SRC_IA%\%3\9*.tga 			continents\%1\collisionmap\
echo Updating zone bitmaps files...
xcopy /D /EXCLUDE:excl.txt %SRC_DATA%\%2\zonebitmaps\*.tga 	continents\%1\zonebitmaps\
echo Updating zone ligos files...
xcopy /D /EXCLUDE:excl.txt %SRC_DATA%\%2\zoneLigos\*.* 		continents\%1\zoneLigos\
echo Updating zones files...
REM VL xcopy /D /EXCLUDE:excl.txt %SRC_DATA%\%2\zones\*.* 		continents\%1\zones\
if %RECUP_PACS%_t==true_t (
	echo Updating collision maps files...
	xcopy /D /EXCLUDE:excl.txt %SRC_PACS%\%1\pacs\*.* 		continents\%1\pacs\
) else (
	echo Cleaning unused pacs prim directory
	IF EXIST continents\%1\pacs	rd /S /Q continents\%1\pacs\
)
echo Updating land file...
xcopy /D /EXCLUDE:excl.txt %SRC_DATA%\%2\%1.land 		continents\%1\

goto :EOF
