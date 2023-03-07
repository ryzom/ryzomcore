@echo off
title NGINX
echo NGINX on http://localhost:9042/
if /I "%RC_ROOT%"=="" call ..\.nel\path_config.bat
copy config.inc.php .\phpmyadmin\config.inc.php
cd nginx
(
echo worker_processes  1;
echo events {
echo     worker_connections  1024;
echo }
echo http {
echo     include       mime.types;
echo     default_type  application/octet-stream;
echo     sendfile        on;
echo     keepalive_timeout  65;
echo     server {
echo         listen       9042;
echo         server_name  localhost;
echo         root         %RC_ROOT_FW%/code/web/public_php;
echo         index        index.php index.html index.htm;
echo         location ~ /\.ht {
echo             deny  all;
echo         }
echo         location /phpmyadmin {
echo             root  %RC_ROOT_FW%/external;
echo             location ~ ^^/phpmyadmin/^(.+\.php^)$ {
echo                 fastcgi_pass   127.0.0.1:9041;
echo                 fastcgi_index  index.php;
echo                 fastcgi_param  SCRIPT_FILENAME    $document_root$fastcgi_script_name;
echo                 include        fastcgi_params;
echo             }
echo         }
echo         location ~ \.php$ {
echo             fastcgi_pass   127.0.0.1:9041;
echo             fastcgi_index  index.php;
echo             fastcgi_param  SCRIPT_FILENAME    $document_root$fastcgi_script_name;
echo             include        fastcgi_params;
echo         }
echo     }
echo }
)> .\conf\nginx.conf
@echo on
rem nginx
cd ..
