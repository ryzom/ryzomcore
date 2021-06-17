call ..\path_config.bat
set PATH=%RC_PYTHON2%;%PATH%
python web_config.py
cd mariadb
.\bin\mysql -P 9040 -u root < ..\web_config_local.sql
cd ..
pause
