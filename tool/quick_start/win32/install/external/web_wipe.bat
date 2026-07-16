call web_off
cd.>.\nginx\logs\access.log
cd.>.\nginx\logs\error.log
cd.>.\nginx\conf\nginx.conf
rmdir /S /Q ..\pipeline\shard_dev\mysql
mkdir ..\pipeline\shard_dev\mysql
del ..\code\web\public_php\ams\is_installed
del ..\code\web\public_php\config.php
del ..\code\web\public_php\is_installed
del ..\code\web\public_php\db_version_lib
del ..\code\web\public_php\db_version_shard
del ..\code\web\public_php\db_version_tool
del ..\code\web\public_php\db_version_web
del ..\code\web\public_php\db_version_ring
del ..\code\web\public_php\role_service
del ..\code\web\public_php\role_support
del ..\code\web\public_php\role_admin
del ..\code\web\public_php\role_domain
del ..\code\web\public_php\config_user.php
del .\phpmyadmin\config.inc.php
del .\web_config_local.sql
