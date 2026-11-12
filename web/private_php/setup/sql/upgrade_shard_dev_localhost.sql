-- The development shard binds its service ports and the database to 127.0.0.1 (BindAddress in the patchman
-- dev domain, --bind-address of MariaDB), where configure_shard_dev.sql used to write the host name of this
-- machine. Only the rows of the development domain that still hold exactly what it wrote for this machine are
-- changed; the addresses the clients get keep the host name (web host, WsAddr).
-- The web host is host:port, the login page puts the scheme in front: the old script wrote it with the scheme,
-- and the clients got http://http://... for the Ring pages.
USE `nel`;
UPDATE `domain` SET `login_address` = '127.0.0.1:46993' WHERE `domain_id` = 90 AND `domain_name` = 'dev' AND `login_address` = '%RC_HOSTNAME%:46993';
UPDATE `domain` SET `session_manager_address` = '127.0.0.1:46994' WHERE `domain_id` = 90 AND `domain_name` = 'dev' AND `session_manager_address` = '%RC_HOSTNAME%:46994';
UPDATE `domain` SET `web_host` = '%RC_HOSTNAME%:9042' WHERE `domain_id` = 90 AND `domain_name` = 'dev' AND `web_host` = 'http://%RC_HOSTNAME%:9042';
UPDATE `domain` SET `web_host_php` = '%RC_HOSTNAME%:9042' WHERE `domain_id` = 90 AND `domain_name` = 'dev' AND `web_host_php` = 'http://%RC_HOSTNAME%:9042';

USE `nel_tool`;
UPDATE `neltool_domains` SET `domain_as_host` = '127.0.0.1' WHERE `domain_id` = 90 AND `domain_name` = 'dev' AND `domain_as_host` = '%RC_HOSTNAME%';
UPDATE `neltool_domains` SET `domain_sql_string` = 'mysql://root@127.0.0.1:9040/ring_dev' WHERE `domain_id` = 90 AND `domain_name` = 'dev' AND `domain_sql_string` = 'mysql://root@%RC_HOSTNAME%:9040/ring_dev';
