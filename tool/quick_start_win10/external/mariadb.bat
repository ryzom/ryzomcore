@echo off
cd mariadb
title MariaDB
echo MariaDB on localhost:9040
echo Launch `cmd_tools.bat` and run `mysql --port=9040` to connect
@echo on
if not exist ..\..\pipeline\shard_dev\mysql\my.ini .\bin\mysql_install_db --default-user --datadir=..\..\pipeline\shard_dev\mysql
.\bin\mysqld --no-defaults --port=9040 --datadir=..\..\pipeline\shard_dev\mysql --character-set-server=utf8mb4 --collation-server=utf8mb4_general_ci --console --standalone
