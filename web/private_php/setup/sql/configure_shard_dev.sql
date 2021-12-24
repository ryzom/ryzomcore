-- Add the domain to the NeL database
USE `nel`;
INSERT INTO `domain` (`domain_id`, `domain_name`, `status`, `patch_version`, `backup_patch_url`, `patch_urls`, `login_address`, `session_manager_address`, `ring_db_name`, `web_host`, `web_host_php`, `description`)
	VALUES ('90', 'dev', 'ds_open', '1', NULL, NULL, '%RC_HOSTNAME%:46993', '%RC_HOSTNAME%:46994', 'ring_dev', 'http://%RC_HOSTNAME%:9042', 'http://%RC_HOSTNAME%:9042', 'Development Domain');

-- Add the domain to the NeL Admin Tool database
USE `nel_tool`;
INSERT INTO `neltool_domains` (`domain_id`, `domain_name`, `domain_as_host`, `domain_as_port`, `domain_rrd_path`, `domain_las_admin_path`, `domain_las_local_path`, `domain_application`, `domain_sql_string`, `domain_hd_check`, `domain_mfs_web`, `domain_cs_sql_string`) VALUES ('90', 'dev', '%RC_HOSTNAME%', '46999', '%RC_SHARD_DEV%/rrd_graphs', '', '', 'dev', 'mysql://root@%RC_HOSTNAME%:9040/ring_dev', '0', NULL, NULL);

-- Add the mainland shard to the NeL database
use `nel`;
INSERT INTO `shard` (`ShardId`, `domain_id`, `WsAddr`, `Name`, `State`, `MOTD`) VALUES ('901', '90', '%RC_HOSTNAME%', 'mainland', 'ds_open', '');

-- Add the mainland and ring shards to the Ring database
use `ring_dev`;
INSERT INTO `sessions` (`session_id`, `session_type`, `title`, `description`) VALUES ('901', 'st_mainland', 'Mainland', '');
INSERT INTO `shard` (`shard_id`, `RequiredState`, `MOTD`) VALUES ('901', 'ds_open', '');
INSERT INTO `shard` (`shard_id`, `RequiredState`, `MOTD`) VALUES ('911', 'ds_open', '');

-- Add the shards to the Admin Tool database
USE `nel_tool`;
INSERT INTO `neltool_shards` (`shard_id`, `shard_name`, `shard_as_id`, `shard_domain_id`, `shard_lang`, `shard_restart`) VALUES ('900', 'Unifier', 'unifier', '90', 'en', '0');
INSERT INTO `neltool_shards` (`shard_id`, `shard_name`, `shard_as_id`, `shard_domain_id`, `shard_lang`, `shard_restart`) VALUES ('901', 'Mainland', 'mainland', '90', 'en', '0');
INSERT INTO `neltool_shards` (`shard_id`, `shard_name`, `shard_as_id`, `shard_domain_id`, `shard_lang`, `shard_restart`) VALUES ('911', 'Ring', 'ring', '90', 'en', '0');

-- Grant admin tool access to the domain
USE `nel_tool`;
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (1, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (2, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (3, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (4, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (5, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (6, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (7, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (8, 90);
INSERT INTO `neltool_group_domains` (`group_domain_group_id`, `group_domain_domain_id`) VALUES (9, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (1, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (2, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (3, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (4, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (5, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (6, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (7, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (8, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (9, 900, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (1, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (2, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (3, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (4, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (5, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (6, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (7, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (8, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (9, 901, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (1, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (2, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (3, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (4, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (5, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (6, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (7, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (8, 911, 90);
INSERT INTO `neltool_group_shards` (`group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES (9, 911, 90);
UPDATE `neltool_groups` SET `group_default_domain_id` = '90', `group_default_shard_id` = '901';
