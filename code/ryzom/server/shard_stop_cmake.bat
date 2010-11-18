@echo off

REM This script will kill all the services launched by shard_start.bat

rem AS
taskkill /IM ryzom_admin_service.exe

rem  bms_master
taskkill /IM ryzom_backup_service.exe

rem  egs
taskkill /IM ryzom_entities_game_service.exe

rem  gpms
taskkill /IM ryzom_gpm_service.exe

rem  ios
taskkill /IM ryzom_ios_service.exe

rem  rns
taskkill /IM ryzom_naming_service.exe

rem  rws
taskkill /IM ryzom_welcome_service.exe

rem  ts
taskkill /IM ryzom_tick_service.exe

rem  ms
taskkill /IM ryzom_mirror_service.exe

rem  ais_newbyland
taskkill /IM ryzom_ai_service.exe

rem  mfs
taskkill /IM ryzom_mail_forum_service.exe

rem  su
taskkill /IM ryzom_shard_unifier_service.exe

rem  fes
taskkill /IM ryzom_frontend_service.exe

rem  sbs
taskkill /IM ryzom_session_browser_server.exe

rem  lgs
taskkill /IM ryzom_logger_service.exe

rem  dss
taskkill /IM ryzom_dynamic_scenario_service.exe

rem  ras
taskkill /IM ryzom_admin_service.exe
