@echo off
rem Wipes the development web of this root: stops it, and removes the database, the web configuration of this
rem root (RC_WEB_DIR) and the configuration the web setup wrote into code\web\public_php
call "%~dp0web_off.bat"
rmdir /S /Q "%RC_ROOT%\pipeline\shard_dev\mysql"
rmdir /S /Q "%RC_WEB_DIR%"
del "%RC_CODE_DIR%\web\public_php\config.php"
del "%RC_CODE_DIR%\web\public_php\is_installed"
del "%RC_CODE_DIR%\web\public_php\db_version_lib"
del "%RC_CODE_DIR%\web\public_php\db_version_shard"
del "%RC_CODE_DIR%\web\public_php\db_version_tool"
del "%RC_CODE_DIR%\web\public_php\db_version_web"
del "%RC_CODE_DIR%\web\public_php\db_version_ring"
del "%RC_CODE_DIR%\web\public_php\role_service"
del "%RC_CODE_DIR%\web\public_php\role_support"
del "%RC_CODE_DIR%\web\public_php\role_admin"
del "%RC_CODE_DIR%\web\public_php\role_domain"
del "%RC_CODE_DIR%\web\public_php\config_user.php"
