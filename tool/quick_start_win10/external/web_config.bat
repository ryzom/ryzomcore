call ..\path_config.bat
python web_config.py
cd mariadb
.\bin\mysql --port=9040 -u=root ..\web_config_local.sql
cd ..
