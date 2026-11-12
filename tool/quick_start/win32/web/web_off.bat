@echo off
rem Stops the development web stack of this root, when it runs outside of its service dashboard
"%RC_NGINX_DIR%\nginx" -p "%RC_WEB_DIR%\nginx/" -c conf/nginx.conf -s stop
set RC_WEB_WAIT=0
:nginxwait
set /a RC_WEB_WAIT+=1
if %RC_WEB_WAIT% gtr 50 goto :nginxkill
"%RC_UTILS_DIR%\nircmd.exe" wait 100
for /F %%x in ('tasklist /NH /FI "IMAGENAME eq nginx.exe"') do if %%x == nginx.exe goto :nginxwait
:nginxkill
taskkill /f /im nginx.exe
taskkill /f /im php-cgi.exe
for /F %%x in ('tasklist /NH /FI "IMAGENAME eq mysqld.exe"') do if %%x == mysqld.exe goto :mariadbfound
goto :mariadbnotfound
:mariadbfound
"%RC_MARIADB_DIR%\bin\mysqladmin" -u root -P 9040 shutdown
set RC_WEB_WAIT=0
:mariadbwait
set /a RC_WEB_WAIT+=1
if %RC_WEB_WAIT% gtr 100 goto :mariadbtimeout
"%RC_UTILS_DIR%\nircmd.exe" wait 100
for /F %%x in ('tasklist /NH /FI "IMAGENAME eq mysqld.exe"') do if %%x == mysqld.exe goto :mariadbwait
goto :mariadbnotfound
:mariadbtimeout
echo WARNING: A mysqld.exe is still running after the shutdown of the database on port 9040
:mariadbnotfound
