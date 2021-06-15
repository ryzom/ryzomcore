@echo off
cd php
title PHP-CGI
echo PHP-CGI on 127.0.0.1:9041
@echo on
php-cgi.exe -b 127.0.0.1:9041
