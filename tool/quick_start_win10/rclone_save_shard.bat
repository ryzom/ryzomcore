call path_config.bat
rem %RC_ROOT%\distribution\utils\rclone.exe sync core4:core4/save_shard_lgs "%RC_ROOT%\save\save_shard_lgs" --transfers=128 --verbose
rem %RC_ROOT%\distribution\utils\rclone.exe sync core4:core4/save_shard_bs "%RC_ROOT%\save\save_shard_bs" --transfers=128 --verbose
rem %RC_ROOT%\distribution\utils\rclone.exe sync core4:core4/www "%RC_ROOT%\save\www" --transfers=128 --verbose
rem rem todo make a zip file because otherwise this is too many writes to b2
rem rem %RC_ROOT%\distribution\utils\rclone.exe sync %RC_ROOT%\save b2:core4-r/save --fast-list --transfers=16 --verbose
pause
