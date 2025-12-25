# syntax=docker/dockerfile:1.20
FROM serversideup/php:8.4-fpm-apache

COPY --chown=www-data:www-data --parents docs private_php /var/www/html/
COPY --chown=www-data:www-data public_php /var/www/html/public/
