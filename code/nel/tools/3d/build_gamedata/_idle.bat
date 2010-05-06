@echo off

REM PAUSE ?
IF EXIST c:\pause.tag (
del c:\pause.tag
echo BUILD PAUSED
pause
echo BUILD RESUMED
)

REM -- Check if the database is connected...

@echo off
IF EXIST database_directory (goto end)
	echo "Waiting database connection on database_directory..."
	echo "ERROR : Waiting database connection on database_directory..." >> log.log
:again
	sleep 10
	net use database_letter database_server
	@IF EXIST database_directory (goto end) ELSE (goto again)
:end
