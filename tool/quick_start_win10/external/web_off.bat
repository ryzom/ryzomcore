@echo off
cd nginx
.\nginx -s stop
cd ..
:nginxwait
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
for /F %%x in ('tasklist /NH /FI "IMAGENAME eq nginx.exe"') do if %%x == nginx.exe goto :nginxwait
taskkill /f /im nginx.exe
taskkill /f /im php-cgi.exe
for /F %%x in ('tasklist /NH /FI "IMAGENAME eq mysqld.exe"') do if %%x == mysqld.exe goto :mariadbfound
taskkill /f /im mysqld.exe
goto :mariadbnotfound
:mariadbfound
cd mariadb
.\bin\mysqladmin -u root -P 9040 shutdown
cd ..
:mariadbwait
..\distribution\ryzom_tools_win_x64\nircmd.exe wait 100
for /F %%x in ('tasklist /NH /FI "IMAGENAME eq mysqld.exe"') do if %%x == mysqld.exe goto :mariadbwait
:mariadbnotfound
