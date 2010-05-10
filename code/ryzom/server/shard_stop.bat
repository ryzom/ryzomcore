@echo off

REM This script will kill all the services launched by shard_start.bat

rem AS
taskkill /IM ryzom_admin_service.exe

rem  bms_master
taskkill /IM backup_service.exe

rem  egs
taskkill /IM entities_game_service.exe

rem  gpms
taskkill /IM gpm_service.exe

rem  ios
taskkill /IM input_output_service.exe

rem  rns
taskkill /IM ryzom_naming_service.exe

rem  rws
taskkill /IM ryzom_welcome_service.exe

rem  ts
taskkill /IM tick_service.exe

rem  ms
taskkill /IM mirror_service.exe

rem  ais_newbyland
taskkill /IM ai_service.exe

rem  mfs
taskkill /IM mail_forum_service.exe

rem  su
taskkill /IM shard_unifier_service.exe

rem  fes
taskkill /IM frontend_service.exe

rem  sbs
taskkill /IM session_browser_server.exe

rem  lgs
taskkill /IM logger_service.exe

rem  ras
taskkill /IM ryzom_admin_service.exe
