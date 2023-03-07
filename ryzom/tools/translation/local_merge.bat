@echo off
if "%_4ver%" == "" goto not4nt
setlocal

r:
cd \code\ryzom\translation

REM cleanup diff directory
del /Y \code\ryzom\translation\bin\.#*.*
del /Y \code\ryzom\translation\diff\*.*
del /Y \code\ryzom\translation\history\*.*
del /Y \code\ryzom\translation\translated\*.*
del /Y \code\ryzom\translation\work\*.*
del /Y \code\ryzom\translation\.#*.*
del /Y \code\ryzom\translation\*.log

: get lastest cvs clean copy
cvs -z3 update -P -C 

REM First run, translate wk to translation
call :make_translation
REM Second run, translate wk to other languages
call :make_translation

: run the 'install' batch files
for %f in (*install*.bat) (echo.| call %f)

goto :EOF

:make_translation
	: run the 'make' batch files
	echo.| CALL 1_make_phrase_diff.bat
	echo.| CALL 5_make_words_diff.bat
	echo.| CALL A_make_string_diff.bat
	echo.| CALL C_make_bot_names_diff.bat

	call :validate_diffs

	: run the 'merge' batch files
	echo.| CALL 2_merge_phrase_diff.bat
	echo.| CALL 6_merge_words_diff.bat
	echo.| CALL B_merge_string_diff.bat
	echo.| CALL D_merge_bot_names_diff.bat

	REM second run for the clause diff
	echo.| CALL 3_make_clause_diff.bat
	call :validate_diffs
	echo.| CALL 4_merge_clause_diff.bat
goto :EOF

:validate_diffs
	REM remove the last 2 lines from the 'diff' files
	cd diff
	do filename in *.?xt
	  REM : we remove 3 lines because there is a pending new line at end of file that count for 1 line
	  ..\bin\translation_tools.exe crop_lines %filename% 3
	enddo
	cd ..
goto :EOF

:not4nt
\\server\code\bin\4nt /c %0
