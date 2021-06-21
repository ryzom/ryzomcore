@echo off
cd mariadb
title MariaDB
echo MariaDB on localhost:9040
echo Launch `cmd_tools.bat` and run `mysql --port=9040` to connect
@echo on
if not exist ..\..\pipeline\shard_dev\mysql\my.ini goto :setup
goto :done
:setup
.\bin\mysql_install_db --default-user --datadir=..\..\pipeline\shard_dev\mysql
if %errorlevel% neq 0 pause
if not exist ..\phpmyadmin.sql goto :done
start .\bin\mysqld --no-defaults --port=9040 --datadir=..\..\pipeline\shard_dev\mysql --character-set-server=utf8mb4 --collation-server=utf8mb4_general_ci --standalone
if %errorlevel% neq 0 pause
.\bin\mysql -P 9040 -u root < ..\phpmyadmin.sql
if %errorlevel% neq 0 pause
.\bin\mysqladmin -u root -P 9040 shutdown
if %errorlevel% neq 0 pause
:done
rem .\bin\mysqld --no-defaults --port=9040 --datadir=..\..\pipeline\shard_dev\mysql --character-set-server=utf8mb4 --collation-server=utf8mb4_general_ci --console --standalone
cd ..
