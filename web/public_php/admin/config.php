<?php

	require_once('../config.php');

	define('NELTOOL_LOADED', true);

	// database information for nel tool
	define('NELTOOL_DBHOST', $cfg['db']['tool']['host']);
	define('NELTOOL_DBUSER', $cfg['db']['tool']['user']);
	define('NELTOOL_DBPASS', $cfg['db']['tool']['pass']);
	define('NELTOOL_DBNAME', $cfg['db']['tool']['name']);

	// site paths definitions
	define('NELTOOL_SITEBASE',  dirname($_SERVER['PHP_SELF']) .'/');
	define('NELTOOL_SYSTEMBASE', dirname(__FILE__) .'/');
	define('NELTOOL_LOGBASE', NELTOOL_SYSTEMBASE .'/logs/');
	define('NELTOOL_IMGBASE', NELTOOL_SYSTEMBASE .'/imgs/');

    define('NELTOOL_RRDTOOL', '/usr/bin/rrdtool');
    define('NELTOOL_RRDSYSBASE', NELTOOL_SYSTEMBASE . 'graphs_output/');
    define('NELTOOL_RRDWEBBASE', NELTOOL_SITEBASE . 'graphs_output/');

	define('NELTOOL_SITETITLE', 'Ryzom Core Admin');
	define('NELTOOL_SESSIONID', 'sid');

	define('NELTOOL_DEBUG', true);

	// SQL table names
	define('NELDB_PREFIX', 'neltool_');

	// for later use
	// the config table will gather some of the settings
	// that are currently written in this config.php file
	//define('NELDB_CONFIG_TABLE', 				NELDB_PREFIX .'config');

	define('NELDB_USER_TABLE', 					NELDB_PREFIX .'users');
	define('NELDB_GROUP_TABLE', 				NELDB_PREFIX .'groups');

	define('NELDB_LOG_TABLE',					NELDB_PREFIX .'logs');
	define('NELDB_NOTE_TABLE',					NELDB_PREFIX .'notes');

	define('NELDB_STAT_HD_TIME_TABLE',			NELDB_PREFIX .'stats_hd_times');
	define('NELDB_STAT_HD_TABLE',				NELDB_PREFIX .'stats_hd_datas');

	define('NELDB_ANNOTATION_TABLE',			NELDB_PREFIX .'annotations');
	define('NELDB_LOCK_TABLE',					NELDB_PREFIX .'locks');

	define('NELDB_APPLICATION_TABLE',			NELDB_PREFIX .'applications');
	define('NELDB_GROUP_APPLICATION_TABLE',		NELDB_PREFIX .'group_applications');
	define('NELDB_USER_APPLICATION_TABLE',		NELDB_PREFIX .'user_applications');

	define('NELDB_DOMAIN_TABLE',				NELDB_PREFIX .'domains');
	define('NELDB_USER_DOMAIN_TABLE',			NELDB_PREFIX .'user_domains');
	define('NELDB_GROUP_DOMAIN_TABLE',			NELDB_PREFIX .'group_domains');

	define('NELDB_SHARD_TABLE',					NELDB_PREFIX .'shards');
	define('NELDB_USER_SHARD_TABLE',			NELDB_PREFIX .'user_shards');
	define('NELDB_GROUP_SHARD_TABLE',			NELDB_PREFIX .'group_shards');

	define('NELDB_RESTART_GROUP_TABLE',			NELDB_PREFIX .'restart_groups');
	define('NELDB_RESTART_MESSAGE_TABLE',		NELDB_PREFIX .'restart_messages');
	define('NELDB_RESTART_SEQUENCE_TABLE',		NELDB_PREFIX .'restart_sequences');

	define('VIEW_DELAY', 0);
	define('HARDWARE_REFRESH', 600);
    define('LOCK_TIMEOUT', 1800);
    define('BG_IMG', 'imgs/bg_live.png');

	$nel_user_group_levels	= array(array(	'level_id'		=>	0,
											'level_name'	=>	'Normal'),
									array(	'level_id'		=>	10,
											'level_name'	=>	'Administrator'),
									);

	$restart_notification_emails = array('support@ryzomcore.org');

?>
