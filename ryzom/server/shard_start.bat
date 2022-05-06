@echo off

REM This script will start all the services with good parameters

REM set MODE=Debug
set MODE=Release

rem AS
start %MODE%\ryzom_admin_service.exe --fulladminname=admin_executor_service --shortadminname=AES

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  bms_master
start %MODE%\backup_service --writepid -P49990

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  egs
start %MODE%\entities_game_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  gpms
start %MODE%\gpm_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  ios
start %MODE%\input_output_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  rns
start %MODE%\ryzom_naming_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  rws
start %MODE%\ryzom_welcome_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  ts
start %MODE%\tick_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  ms
start %MODE%\mirror_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  ais_newbyland
start %MODE%\ai_service --writepid -mCommon:Newbieland:Post

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  mfs
start %MODE%\mail_forum_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  su
start %MODE%\shard_unifier_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  fes
start %MODE%\frontend_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  sbs
start %MODE%\session_browser_server --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  lgs
start %MODE%\logger_service --writepid

rem wait 2s (yes, i didn't find a better way to wait N seconds)
ping -n 2 127.0.0.1 > NUL 2>&1

rem  ras
start %MODE%\ryzom_admin_service --fulladminname=admin_service --shortadminname=AS --writepid
