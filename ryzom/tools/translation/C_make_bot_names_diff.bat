@echo off

echo Generating bot_names diff...
bin\translation_tools make_worksheet_diff bot_names.txt
echo Done.
pause