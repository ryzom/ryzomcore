# syntax=docker/dockerfile:1.20
FROM trafex/php-nginx:latest

COPY --parents docs private_php /app/
COPY --exclude=config*.php public_php /var/www/html
