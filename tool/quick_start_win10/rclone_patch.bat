call path_config.bat
rem rem Remote sfo2 is a DigitalOcean Spaces storage with CDN
rem %RC_ROOT%\distribution\utils\rclone.exe copy "%RC_ROOT%\pipeline\client_patch\patch" sfo2:ryzom/core4/patch --verbose
rem if %errorlevel% neq 0 pause
rem rem Remote core4 is an SFTP server running the patchman bridge server
rem rem ^> rclone config
rem rem ^> n
rem rem ^> core4
rem rem ^> core
rem rem ^> sftp
rem rem ^> core.ryzom.dev
rem %RC_ROOT%\distribution\utils\rclone.exe sync "%RC_ROOT%\pipeline\bridge_server" core4:bridge_server --verbose --exclude /.patchman.file_index
pause
