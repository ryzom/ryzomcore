@echo off
rem Creates the development database of this root in pipeline\shard_dev\mysql the first time, with the MariaDB of
rem the externals package (RC_MARIADB_DIR): phpMyAdmin tables, no anonymous users, and the shard user
title MariaDB
echo MariaDB on localhost:9040
echo Launch `cmd_tools.bat` and run `mysql --port=9040` to connect
set "RC_MYSQL_DATA=%RC_ROOT%\pipeline\shard_dev\mysql"
if exist "%RC_MYSQL_DATA%\my.ini" goto :done
@echo on
"%RC_MARIADB_DIR%\bin\mysql_install_db" --default-user --datadir="%RC_MYSQL_DATA%"
@if %errorlevel% neq 0 pause
start "MariaDB Setup" "%RC_MARIADB_DIR%\bin\mysqld" --no-defaults --bind-address=127.0.0.1 --port=9040 --datadir="%RC_MYSQL_DATA%" --character-set-server=utf8mb4 --collation-server=utf8mb4_general_ci --standalone
@if %errorlevel% neq 0 pause
"%RC_MARIADB_DIR%\bin\mysqladmin" -P 9040 -u root --connect-timeout=2 --wait=30 ping
@if %errorlevel% neq 0 pause
"%RC_MARIADB_DIR%\bin\mysql" -P 9040 -u root < "%~dp0phpmyadmin.sql"
@if %errorlevel% neq 0 pause
"%RC_MARIADB_DIR%\bin\mysql" -P 9040 -u root -e "SELECT GROUP_CONCAT(QUOTE(user),'@',QUOTE(host)) INTO @accounts FROM mysql.user WHERE user = ''; EXECUTE IMMEDIATE IFNULL(CONCAT('DROP USER ', @accounts), 'DO 0');"
@if %errorlevel% neq 0 pause
@rem The database only listens on localhost, the shard user is limited to localhost as well
"%RC_MARIADB_DIR%\bin\mysql" -P 9040 -u root -e "CREATE USER 'shard_dev'@'localhost' IDENTIFIED BY '%RC_MYSQL_PASSWORD%'; GRANT SELECT, INSERT, UPDATE, DELETE, FILE ON *.* TO 'shard_dev'@'localhost';"
@if %errorlevel% neq 0 pause
"%RC_MARIADB_DIR%\bin\mysqladmin" -u root -P 9040 shutdown
@if %errorlevel% neq 0 pause
@echo off
:done
