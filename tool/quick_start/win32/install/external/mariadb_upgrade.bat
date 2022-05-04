cd mariadb
rem MariaDB must be running to do this
.\bin\mysql_upgrade --port=9040
echo Now restart web
pause
cd ..
