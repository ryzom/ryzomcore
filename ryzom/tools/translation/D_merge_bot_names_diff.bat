@echo off

echo Merging bot_names diff...
bin\translation_tools merge_worksheet_diff bot_names.txt
echo Done.
pause