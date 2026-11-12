@echo off
rem Upgrades the system tables of the development database after the externals package brings a newer MariaDB.
rem MariaDB must be running to do this (web_on.bat)
"%RC_MARIADB_DIR%\bin\mysql_upgrade" -u root --port=9040
echo Now restart web
pause
