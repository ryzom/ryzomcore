# --------------------------------------------------------
# Host:                         94.23.202.75
# Database:                     nel
# Server version:               5.1.37-1ubuntu5.1
# Server OS:                    debian-linux-gnu
# HeidiSQL version:             5.0.0.3272
# Date/time:                    2010-05-08 09:14:27
# --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
# Dumping database structure for nel
CREATE DATABASE IF NOT EXISTS `nel` /*!40100 DEFAULT CHARACTER SET latin1 */;
USE `nel`;


# Dumping structure for table nel.domain
CREATE TABLE IF NOT EXISTS `domain` (
  `domain_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `domain_name` varchar(32) NOT NULL DEFAULT '',
  `status` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev',
  `patch_version` int(10) unsigned NOT NULL DEFAULT '0',
  `backup_patch_url` varchar(255) DEFAULT NULL,
  `patch_urls` text,
  `login_address` varchar(255) NOT NULL DEFAULT '',
  `session_manager_address` varchar(255) NOT NULL DEFAULT '',
  `ring_db_name` varchar(255) NOT NULL DEFAULT '',
  `web_host` varchar(255) NOT NULL DEFAULT '',
  `web_host_php` varchar(255) NOT NULL DEFAULT '',
  `description` varchar(200) DEFAULT NULL,
  PRIMARY KEY (`domain_id`),
  UNIQUE KEY `name_idx` (`domain_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel.permission
CREATE TABLE IF NOT EXISTS `permission` (
  `UId` int(10) unsigned NOT NULL DEFAULT '0',
  `ClientApplication` char(64) NOT NULL DEFAULT 'ryzom',
  `ShardId` int(10) NOT NULL DEFAULT '-1',
  `AccessPrivilege` set('OPEN','DEV','RESTRICTED') NOT NULL DEFAULT 'OPEN',
  `prim` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`prim`),
  KEY `UIDIndex` (`UId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel.shard
CREATE TABLE IF NOT EXISTS `shard` (
  `ShardId` int(10) NOT NULL DEFAULT '0',
  `domain_id` int(11) unsigned NOT NULL DEFAULT '0',
  `WsAddr` varchar(64) DEFAULT NULL,
  `NbPlayers` int(10) unsigned DEFAULT '0',
  `Name` varchar(255) DEFAULT 'unknown shard',
  `Online` tinyint(1) unsigned DEFAULT '0',
  `ClientApplication` varchar(64) DEFAULT 'ryzom',
  `Version` varchar(64) NOT NULL DEFAULT '',
  `PatchURL` varchar(255) NOT NULL DEFAULT '',
  `DynPatchURL` varchar(255) NOT NULL DEFAULT '',
  `FixedSessionId` int(11) unsigned NOT NULL DEFAULT '0',
  `State` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev',
  `MOTD` text NOT NULL,
  `prim` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`prim`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all shards information for login system';

# Data exporting was unselected.


# Dumping structure for table nel.user
CREATE TABLE IF NOT EXISTS `user` (
  `UId` int(10) NOT NULL AUTO_INCREMENT,
  `Login` varchar(64) NOT NULL DEFAULT '',
  `Password` varchar(13) DEFAULT NULL,
  `ShardId` int(10) NOT NULL DEFAULT '-1',
  `State` enum('Offline','Online') NOT NULL DEFAULT 'Offline',
  `Privilege` varchar(255) NOT NULL DEFAULT '',
  `GroupName` varchar(255) NOT NULL DEFAULT '',
  `FirstName` varchar(255) NOT NULL DEFAULT '',
  `LastName` varchar(255) NOT NULL DEFAULT '',
  `Birthday` varchar(32) NOT NULL DEFAULT '',
  `Gender` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `Country` char(2) NOT NULL DEFAULT '',
  `Email` varchar(255) NOT NULL DEFAULT '',
  `Address` varchar(255) NOT NULL DEFAULT '',
  `City` varchar(100) NOT NULL DEFAULT '',
  `PostalCode` varchar(10) NOT NULL DEFAULT '',
  `USState` char(2) NOT NULL DEFAULT '',
  `Chat` char(2) NOT NULL DEFAULT '0',
  `BetaKeyId` int(10) unsigned NOT NULL DEFAULT '0',
  `CachedCoupons` varchar(255) NOT NULL DEFAULT '',
  `ProfileAccess` varchar(45) DEFAULT NULL,
  `Level` int(2) NOT NULL DEFAULT '0',
  `CurrentFunds` int(4) NOT NULL DEFAULT '0',
  `IdBilling` varchar(255) NOT NULL DEFAULT '',
  `Community` char(2) NOT NULL DEFAULT '--',
  `Newsletter` tinyint(1) NOT NULL DEFAULT '1',
  `Account` varchar(64) NOT NULL DEFAULT '',
  `ChoiceSubLength` tinyint(2) NOT NULL DEFAULT '0',
  `CurrentSubLength` varchar(255) NOT NULL DEFAULT '0',
  `ValidIdBilling` int(4) NOT NULL DEFAULT '0',
  `GMId` int(4) NOT NULL DEFAULT '0',
  `ExtendedPrivilege` varchar(128) NOT NULL DEFAULT '',
  `ToolsGroup` varchar(20) NOT NULL DEFAULT '',
  `Unsubscribe` date NOT NULL DEFAULT '0000-00-00',
  `SubDate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `SubIp` varchar(20) NOT NULL DEFAULT '',
  `SecurePassword` varchar(32) NOT NULL DEFAULT '',
  `LastInvoiceEmailCheck` date NOT NULL DEFAULT '0000-00-00',
  `FromSource` varchar(8) NOT NULL DEFAULT '',
  `ValidMerchantCode` varchar(13) NOT NULL DEFAULT '',
  `PBC` tinyint(1) NOT NULL DEFAULT '0',
  `ApiKeySeed` varchar(8) DEFAULT NULL,
  PRIMARY KEY (`UId`),
  KEY `LoginIndex` (`Login`),
  KEY `GroupIndex` (`GroupName`),
  KEY `ToolsGroup` (`ToolsGroup`),
  KEY `CurrentSubLength` (`CurrentSubLength`),
  KEY `Community` (`Community`),
  KEY `Email` (`Email`),
  KEY `GMId` (`GMId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users information for login system';

# Data exporting was unselected.
# --------------------------------------------------------
# Host:                         94.23.202.75
# Database:                     nel_tool
# Server version:               5.1.37-1ubuntu5.1
# Server OS:                    debian-linux-gnu
# HeidiSQL version:             5.0.0.3272
# Date/time:                    2010-05-08 09:14:28
# --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
# Dumping database structure for nel_tool
CREATE DATABASE IF NOT EXISTS `nel_tool` /*!40100 DEFAULT CHARACTER SET latin1 */;
USE `nel_tool`;


# Dumping structure for table nel_tool.neltool_annotations
CREATE TABLE IF NOT EXISTS `neltool_annotations` (
  `annotation_id` int(11) NOT NULL AUTO_INCREMENT,
  `annotation_domain_id` int(11) DEFAULT NULL,
  `annotation_shard_id` int(11) DEFAULT NULL,
  `annotation_data` varchar(255) NOT NULL DEFAULT '',
  `annotation_user_name` varchar(32) NOT NULL DEFAULT '',
  `annotation_date` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`annotation_id`),
  UNIQUE KEY `annotation_shard_id` (`annotation_shard_id`),
  UNIQUE KEY `annotation_domain_id` (`annotation_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_applications
CREATE TABLE IF NOT EXISTS `neltool_applications` (
  `application_id` int(11) NOT NULL AUTO_INCREMENT,
  `application_name` varchar(64) NOT NULL DEFAULT '',
  `application_uri` varchar(255) NOT NULL DEFAULT '',
  `application_restriction` varchar(64) NOT NULL DEFAULT '',
  `application_order` int(11) NOT NULL DEFAULT '0',
  `application_visible` int(11) NOT NULL DEFAULT '0',
  `application_icon` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`application_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_domains
CREATE TABLE IF NOT EXISTS `neltool_domains` (
  `domain_id` int(11) NOT NULL AUTO_INCREMENT,
  `domain_name` varchar(128) NOT NULL DEFAULT '',
  `domain_as_host` varchar(128) NOT NULL DEFAULT '',
  `domain_as_port` int(11) NOT NULL DEFAULT '0',
  `domain_rrd_path` varchar(255) NOT NULL DEFAULT '',
  `domain_las_admin_path` varchar(255) NOT NULL DEFAULT '',
  `domain_las_local_path` varchar(255) NOT NULL DEFAULT '',
  `domain_application` varchar(128) NOT NULL DEFAULT '',
  `domain_sql_string` varchar(128) NOT NULL DEFAULT '',
  `domain_hd_check` int(11) NOT NULL DEFAULT '0',
  `domain_mfs_web` text,
  `domain_cs_sql_string` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_groups
CREATE TABLE IF NOT EXISTS `neltool_groups` (
  `group_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_name` varchar(32) NOT NULL DEFAULT 'NewGroup',
  `group_level` int(11) NOT NULL DEFAULT '0',
  `group_default` int(11) NOT NULL DEFAULT '0',
  `group_active` int(11) NOT NULL DEFAULT '0',
  `group_default_domain_id` tinyint(3) unsigned DEFAULT NULL,
  `group_default_shard_id` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_group_applications
CREATE TABLE IF NOT EXISTS `neltool_group_applications` (
  `group_application_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_application_group_id` int(11) NOT NULL DEFAULT '0',
  `group_application_application_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`group_application_id`),
  KEY `group_application_group_id` (`group_application_group_id`),
  KEY `group_application_application_id` (`group_application_application_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_group_domains
CREATE TABLE IF NOT EXISTS `neltool_group_domains` (
  `group_domain_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_domain_group_id` int(11) NOT NULL DEFAULT '0',
  `group_domain_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`group_domain_id`),
  KEY `group_domain_group_id` (`group_domain_group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_group_shards
CREATE TABLE IF NOT EXISTS `neltool_group_shards` (
  `group_shard_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_shard_group_id` int(11) NOT NULL DEFAULT '0',
  `group_shard_shard_id` int(11) NOT NULL DEFAULT '0',
  `group_shard_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`group_shard_id`),
  KEY `group_shard_group_id` (`group_shard_group_id`),
  KEY `group_shard_domain_id` (`group_shard_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_locks
CREATE TABLE IF NOT EXISTS `neltool_locks` (
  `lock_id` int(11) NOT NULL AUTO_INCREMENT,
  `lock_domain_id` int(11) DEFAULT NULL,
  `lock_shard_id` int(11) DEFAULT NULL,
  `lock_user_name` varchar(32) NOT NULL DEFAULT '',
  `lock_date` int(11) NOT NULL DEFAULT '0',
  `lock_update` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`lock_id`),
  UNIQUE KEY `lock_shard_id` (`lock_shard_id`),
  UNIQUE KEY `lock_domain_id` (`lock_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_logs
CREATE TABLE IF NOT EXISTS `neltool_logs` (
  `logs_id` int(11) NOT NULL AUTO_INCREMENT,
  `logs_user_name` varchar(32) NOT NULL DEFAULT '0',
  `logs_date` int(11) NOT NULL DEFAULT '0',
  `logs_data` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`logs_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_notes
CREATE TABLE IF NOT EXISTS `neltool_notes` (
  `note_id` int(11) NOT NULL AUTO_INCREMENT,
  `note_user_id` int(11) NOT NULL DEFAULT '0',
  `note_title` varchar(128) NOT NULL DEFAULT '',
  `note_data` text NOT NULL,
  `note_date` int(11) NOT NULL DEFAULT '0',
  `note_active` int(11) NOT NULL DEFAULT '0',
  `note_global` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`note_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_restart_groups
CREATE TABLE IF NOT EXISTS `neltool_restart_groups` (
  `restart_group_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `restart_group_name` varchar(50) DEFAULT NULL,
  `restart_group_list` varchar(50) DEFAULT NULL,
  `restart_group_order` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`restart_group_id`),
  UNIQUE KEY `restart_group_id` (`restart_group_id`),
  KEY `restart_group_id_2` (`restart_group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_restart_messages
CREATE TABLE IF NOT EXISTS `neltool_restart_messages` (
  `restart_message_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `restart_message_name` varchar(20) DEFAULT NULL,
  `restart_message_value` varchar(128) DEFAULT NULL,
  `restart_message_lang` varchar(5) DEFAULT NULL,
  PRIMARY KEY (`restart_message_id`),
  UNIQUE KEY `restart_message_id` (`restart_message_id`),
  KEY `restart_message_id_2` (`restart_message_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_restart_sequences
CREATE TABLE IF NOT EXISTS `neltool_restart_sequences` (
  `restart_sequence_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `restart_sequence_domain_id` int(10) unsigned NOT NULL DEFAULT '0',
  `restart_sequence_shard_id` int(10) unsigned NOT NULL DEFAULT '0',
  `restart_sequence_user_name` varchar(50) DEFAULT NULL,
  `restart_sequence_step` int(10) unsigned NOT NULL DEFAULT '0',
  `restart_sequence_date_start` int(11) DEFAULT NULL,
  `restart_sequence_date_end` int(11) DEFAULT NULL,
  `restart_sequence_timer` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`restart_sequence_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_shards
CREATE TABLE IF NOT EXISTS `neltool_shards` (
  `shard_id` int(11) NOT NULL AUTO_INCREMENT,
  `shard_name` varchar(128) NOT NULL DEFAULT '',
  `shard_as_id` varchar(255) NOT NULL DEFAULT '0',
  `shard_domain_id` int(11) NOT NULL DEFAULT '0',
  `shard_lang` char(2) NOT NULL DEFAULT 'en',
  `shard_restart` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`shard_id`),
  KEY `shard_domain_id` (`shard_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_stats_hd_datas
CREATE TABLE IF NOT EXISTS `neltool_stats_hd_datas` (
  `hd_id` int(11) NOT NULL AUTO_INCREMENT,
  `hd_domain_id` int(11) NOT NULL DEFAULT '0',
  `hd_server` varchar(32) NOT NULL DEFAULT '',
  `hd_device` varchar(64) NOT NULL DEFAULT '',
  `hd_size` varchar(16) NOT NULL DEFAULT '',
  `hd_used` varchar(16) NOT NULL DEFAULT '',
  `hd_free` varchar(16) NOT NULL DEFAULT '',
  `hd_percent` int(11) NOT NULL DEFAULT '0',
  `hd_mount` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`hd_id`),
  KEY `hd_domain_id` (`hd_domain_id`),
  KEY `hd_server` (`hd_server`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_stats_hd_times
CREATE TABLE IF NOT EXISTS `neltool_stats_hd_times` (
  `hd_domain_id` int(11) NOT NULL DEFAULT '0',
  `hd_last_time` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`hd_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_users
CREATE TABLE IF NOT EXISTS `neltool_users` (
  `user_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(32) NOT NULL DEFAULT '',
  `user_password` varchar(64) NOT NULL DEFAULT '',
  `user_group_id` int(11) NOT NULL DEFAULT '0',
  `user_created` int(11) NOT NULL DEFAULT '0',
  `user_active` int(11) NOT NULL DEFAULT '0',
  `user_logged_last` int(11) NOT NULL DEFAULT '0',
  `user_logged_count` int(11) NOT NULL DEFAULT '0',
  `user_menu_style` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_login` (`user_name`),
  KEY `user_group_id` (`user_group_id`),
  KEY `user_active` (`user_active`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_user_applications
CREATE TABLE IF NOT EXISTS `neltool_user_applications` (
  `user_application_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_application_user_id` int(11) NOT NULL DEFAULT '0',
  `user_application_application_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_application_id`),
  KEY `user_application_user_id` (`user_application_user_id`),
  KEY `user_application_application_id` (`user_application_application_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_user_domains
CREATE TABLE IF NOT EXISTS `neltool_user_domains` (
  `user_domain_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_domain_user_id` int(11) NOT NULL DEFAULT '0',
  `user_domain_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_domain_id`),
  KEY `user_domain_user_id` (`user_domain_user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table nel_tool.neltool_user_shards
CREATE TABLE IF NOT EXISTS `neltool_user_shards` (
  `user_shard_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_shard_user_id` int(11) NOT NULL DEFAULT '0',
  `user_shard_shard_id` int(11) NOT NULL DEFAULT '0',
  `user_shard_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_shard_id`),
  KEY `user_shard_user_id` (`user_shard_user_id`),
  KEY `user_shard_domain_id` (`user_shard_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.
# --------------------------------------------------------
# Host:                         94.23.202.75
# Database:                     ring_open
# Server version:               5.1.37-1ubuntu5.1
# Server OS:                    debian-linux-gnu
# HeidiSQL version:             5.0.0.3272
# Date/time:                    2010-05-08 09:14:32
# --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
# Dumping database structure for ring_open
CREATE DATABASE IF NOT EXISTS `ring_open` /*!40100 DEFAULT CHARACTER SET latin1 */;
USE `ring_open`;


# Dumping structure for table ring_open.characters
CREATE TABLE IF NOT EXISTS `characters` (
  `char_id` int(10) unsigned NOT NULL DEFAULT '0',
  `char_name` varchar(20) NOT NULL DEFAULT '',
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `guild_id` int(10) unsigned NOT NULL DEFAULT '0',
  `best_combat_level` int(10) unsigned NOT NULL DEFAULT '0',
  `home_mainland_session_id` int(10) unsigned NOT NULL DEFAULT '0',
  `ring_access` varchar(63) NOT NULL DEFAULT '',
  `race` enum('r_fyros','r_matis','r_tryker','r_zorai') NOT NULL DEFAULT 'r_fyros',
  `civilisation` enum('c_neutral','c_fyros','c_fyros','c_matis','c_tryker','c_zorai') NOT NULL DEFAULT 'c_neutral',
  `cult` enum('c_neutral','c_kami','c_karavan') NOT NULL DEFAULT 'c_neutral',
  `current_session` int(11) unsigned NOT NULL DEFAULT '0',
  `rrp_am` int(11) unsigned NOT NULL DEFAULT '0',
  `rrp_masterless` int(11) unsigned NOT NULL DEFAULT '0',
  `rrp_author` int(11) unsigned NOT NULL DEFAULT '0',
  `newcomer` tinyint(1) NOT NULL DEFAULT '1',
  `creation_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `last_played_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`char_id`),
  UNIQUE KEY `char_name_idx` (`char_name`,`home_mainland_session_id`),
  KEY `user_id_idx` (`user_id`),
  KEY `guild_idx` (`guild_id`),
  KEY `guild_id_idx` (`guild_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table ring_open.folder
CREATE TABLE IF NOT EXISTS `folder` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `owner` int(10) unsigned NOT NULL DEFAULT '0',
  `title` varchar(40) NOT NULL DEFAULT '',
  `comments` text NOT NULL,
  PRIMARY KEY (`Id`),
  KEY `owner_idx` (`owner`),
  KEY `title_idx` (`title`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.folder_access
CREATE TABLE IF NOT EXISTS `folder_access` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `folder_id` int(10) unsigned NOT NULL DEFAULT '0',
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  KEY `folder_id_idx` (`folder_id`),
  KEY `user_idx` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

# Data exporting was unselected.


# Dumping structure for table ring_open.guilds
CREATE TABLE IF NOT EXISTS `guilds` (
  `guild_id` int(10) unsigned NOT NULL DEFAULT '0',
  `guild_name` varchar(50) NOT NULL DEFAULT '',
  `shard_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guild_id`),
  KEY `shard_id_idx` (`shard_id`),
  KEY `guild_name_idx` (`guild_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.guild_invites
CREATE TABLE IF NOT EXISTS `guild_invites` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
  `guild_id` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  KEY `guild_id_idx` (`guild_id`),
  KEY `session_id_idx` (`session_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

# Data exporting was unselected.


# Dumping structure for table ring_open.journal_entry
CREATE TABLE IF NOT EXISTS `journal_entry` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
  `author` int(10) unsigned NOT NULL DEFAULT '0',
  `type` enum('jet_credits','jet_notes') NOT NULL DEFAULT 'jet_notes',
  `text` text NOT NULL,
  `time_stamp` datetime NOT NULL DEFAULT '2005-09-07 12:41:33',
  PRIMARY KEY (`Id`),
  KEY `session_id_idx` (`session_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.known_users
CREATE TABLE IF NOT EXISTS `known_users` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `owner` int(10) unsigned NOT NULL DEFAULT '0',
  `targer_user` int(10) unsigned NOT NULL DEFAULT '0',
  `targer_character` int(10) unsigned NOT NULL DEFAULT '0',
  `relation_type` enum('rt_friend','rt_banned','rt_friend_dm') NOT NULL DEFAULT 'rt_friend',
  `comments` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`Id`),
  KEY `user_index` (`owner`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.mfs_erased_mail_series
CREATE TABLE IF NOT EXISTS `mfs_erased_mail_series` (
  `erased_char_id` int(11) unsigned NOT NULL DEFAULT '0',
  `erased_char_name` varchar(32) NOT NULL DEFAULT '',
  `erased_series` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `erase_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`erased_series`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.mfs_guild_thread
CREATE TABLE IF NOT EXISTS `mfs_guild_thread` (
  `thread_id` int(11) NOT NULL AUTO_INCREMENT,
  `guild_id` int(11) unsigned NOT NULL DEFAULT '0',
  `topic` varchar(255) NOT NULL DEFAULT '',
  `author_name` varchar(32) NOT NULL DEFAULT '',
  `last_post_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `post_count` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`thread_id`),
  KEY `guild_index` (`guild_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.mfs_guild_thread_message
CREATE TABLE IF NOT EXISTS `mfs_guild_thread_message` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `thread_id` int(11) unsigned NOT NULL DEFAULT '0',
  `author_name` varchar(32) NOT NULL DEFAULT '',
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `content` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.mfs_mail
CREATE TABLE IF NOT EXISTS `mfs_mail` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `sender_name` varchar(32) NOT NULL DEFAULT '',
  `subject` varchar(250) NOT NULL DEFAULT '',
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `status` enum('ms_new','ms_read','ms_erased') NOT NULL DEFAULT 'ms_new',
  `dest_char_id` int(11) unsigned NOT NULL DEFAULT '0',
  `erase_series` int(11) unsigned NOT NULL DEFAULT '0',
  `content` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `dest_index` (`dest_char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.outlands
CREATE TABLE IF NOT EXISTS `outlands` (
  `session_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `island_name` text NOT NULL,
  `billing_instance_id` int(11) unsigned NOT NULL DEFAULT '0',
  `anim_session_id` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`session_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

# Data exporting was unselected.


# Dumping structure for table ring_open.player_rating
CREATE TABLE IF NOT EXISTS `player_rating` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `scenario_id` int(10) unsigned NOT NULL DEFAULT '0',
  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
  `rate_fun` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rate_difficulty` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rate_accessibility` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rate_originality` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `rate_direction` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `author` int(10) unsigned NOT NULL DEFAULT '0',
  `rating` int(10) NOT NULL DEFAULT '0',
  `comments` text NOT NULL,
  `time_stamp` datetime NOT NULL DEFAULT '2005-09-07 12:41:33',
  PRIMARY KEY (`Id`),
  KEY `session_id_idx` (`scenario_id`),
  KEY `author_idx` (`author`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.ring_users
CREATE TABLE IF NOT EXISTS `ring_users` (
  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
  `user_name` varchar(20) NOT NULL DEFAULT '',
  `user_type` enum('ut_character','ut_pioneer') NOT NULL DEFAULT 'ut_character',
  `current_session` int(10) unsigned NOT NULL DEFAULT '0',
  `current_activity` enum('ca_none','ca_play','ca_edit','ca_anim') NOT NULL DEFAULT 'ca_none',
  `current_status` enum('cs_offline','cs_logged','cs_online') NOT NULL DEFAULT 'cs_offline',
  `public_level` enum('pl_none','pl_public') NOT NULL DEFAULT 'pl_none',
  `account_type` enum('at_normal','at_gold') NOT NULL DEFAULT 'at_normal',
  `content_access_level` varchar(20) NOT NULL DEFAULT '',
  `description` text NOT NULL,
  `lang` enum('lang_en','lang_fr','lang_de') NOT NULL DEFAULT 'lang_en',
  `cookie` varchar(30) NOT NULL DEFAULT '',
  `current_domain_id` int(10) NOT NULL DEFAULT '-1',
  `pioneer_char_id` int(11) unsigned NOT NULL DEFAULT '0',
  `current_char` int(11) NOT NULL DEFAULT '0',
  `add_privileges` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_name_idx` (`user_name`),
  KEY `cookie_idx` (`cookie`),
  KEY `current_session_idx` (`current_session`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.scenario
CREATE TABLE IF NOT EXISTS `scenario` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `md5` varchar(64) NOT NULL DEFAULT '',
  `title` varchar(32) NOT NULL DEFAULT '',
  `description` text NOT NULL,
  `author` varchar(32) NOT NULL DEFAULT '',
  `rrp_total` int(11) unsigned NOT NULL DEFAULT '0',
  `anim_mode` enum('am_dm','am_autonomous') NOT NULL DEFAULT 'am_dm',
  `language` varchar(11) NOT NULL DEFAULT '',
  `orientation` enum('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') NOT NULL DEFAULT 'so_other',
  `level` enum('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') NOT NULL DEFAULT 'sl_a',
  `allow_free_trial` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.scenario_desc
CREATE TABLE IF NOT EXISTS `scenario_desc` (
  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
  `parent_scenario` int(10) unsigned NOT NULL DEFAULT '0',
  `description` text NOT NULL,
  `relation_to_parent` enum('rtp_same','rtp_variant','rtp_different') NOT NULL DEFAULT 'rtp_same',
  `title` varchar(40) NOT NULL DEFAULT '',
  `num_player` int(10) unsigned NOT NULL DEFAULT '0',
  `content_access_level` varchar(20) NOT NULL DEFAULT '',
  PRIMARY KEY (`session_id`),
  UNIQUE KEY `title_idx` (`title`),
  KEY `parent_idx` (`parent_scenario`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.sessions
CREATE TABLE IF NOT EXISTS `sessions` (
  `session_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `session_type` enum('st_edit','st_anim','st_outland','st_mainland') NOT NULL DEFAULT 'st_edit',
  `title` varchar(40) NOT NULL DEFAULT '',
  `owner` int(10) unsigned NOT NULL DEFAULT '0',
  `plan_date` datetime NOT NULL DEFAULT '2005-09-21 12:41:33',
  `start_date` datetime NOT NULL DEFAULT '2005-08-31 00:00:00',
  `description` text NOT NULL,
  `orientation` enum('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') NOT NULL DEFAULT 'so_other',
  `level` enum('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') NOT NULL DEFAULT 'sl_a',
  `rule_type` enum('rt_strict','rt_liberal') NOT NULL DEFAULT 'rt_strict',
  `access_type` enum('at_public','at_private') NOT NULL DEFAULT 'at_private',
  `state` enum('ss_planned','ss_open','ss_locked','ss_closed') NOT NULL DEFAULT 'ss_planned',
  `host_shard_id` int(11) NOT NULL DEFAULT '0',
  `subscription_slots` int(11) unsigned NOT NULL DEFAULT '0',
  `reserved_slots` int(10) unsigned NOT NULL DEFAULT '0',
  `free_slots` int(10) unsigned NOT NULL DEFAULT '0',
  `estimated_duration` enum('et_short','et_medium','et_long') NOT NULL DEFAULT 'et_short',
  `final_duration` int(10) unsigned NOT NULL DEFAULT '0',
  `folder_id` int(10) unsigned NOT NULL DEFAULT '0',
  `lang` varchar(20) NOT NULL DEFAULT '',
  `icone` varchar(70) NOT NULL DEFAULT '',
  `anim_mode` enum('am_dm','am_autonomous') NOT NULL DEFAULT 'am_dm',
  `race_filter` set('rf_fyros','rf_matis','rf_tryker','rf_zorai') NOT NULL DEFAULT '',
  `religion_filter` set('rf_kami','rf_karavan','rf_neutral') NOT NULL DEFAULT '',
  `guild_filter` enum('gf_only_my_guild','gf_any_player') DEFAULT 'gf_only_my_guild',
  `shard_filter` set('sf_shard00','sf_shard01','sf_shard02','sf_shard03','sf_shard04','sf_shard05','sf_shard06','sf_shard07','sf_shard08','sf_shard09','sf_shard10','sf_shard11','sf_shard12','sf_shard13','sf_shard14','sf_shard15','sf_shard16','sf_shard17','sf_shard18','sf_shard19','sf_shard20','sf_shard21','sf_shard22','sf_shard23','sf_shard24','sf_shard25','sf_shard26','sf_shard27','sf_shard28','sf_shard29','sf_shard30','sf_shard31') NOT NULL DEFAULT 'sf_shard00,sf_shard01,sf_shard02,sf_shard03,sf_shard04,sf_shard05,sf_shard06,sf_shard07,sf_shard08,sf_shard09,sf_shard10,sf_shard11,sf_shard12,sf_shard13,sf_shard14,sf_shard15,sf_shard16,sf_shard17,sf_shard18,sf_shard19,sf_shard20,sf_shard21,sf_shard22,sf_shard23,sf_shard24,sf_shard25,sf_shard26,sf_shard27,sf_shard28,sf_shard29,sf_shard30,sf_shard31',
  `level_filter` set('lf_a','lf_b','lf_c','lf_d','lf_e','lf_f') NOT NULL DEFAULT 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f',
  `subscription_closed` tinyint(1) NOT NULL DEFAULT '0',
  `newcomer` tinyint(1) unsigned zerofill NOT NULL DEFAULT '0',
  PRIMARY KEY (`session_id`),
  KEY `owner_idx` (`owner`),
  KEY `folder_idx` (`folder_id`),
  KEY `state_type_idx` (`state`,`session_type`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.session_log
CREATE TABLE IF NOT EXISTS `session_log` (
  `id` int(11) NOT NULL DEFAULT '0',
  `scenario_id` int(11) unsigned NOT NULL DEFAULT '0',
  `rrp_scored` int(11) unsigned NOT NULL DEFAULT '0',
  `scenario_point_scored` int(11) unsigned NOT NULL DEFAULT '0',
  `time_taken` int(11) unsigned NOT NULL DEFAULT '0',
  `participants` text NOT NULL,
  `launch_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `owner` varchar(32) NOT NULL DEFAULT '0',
  `guild_name` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

# Data exporting was unselected.


# Dumping structure for table ring_open.session_participant
CREATE TABLE IF NOT EXISTS `session_participant` (
  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
  `char_id` int(10) unsigned NOT NULL DEFAULT '0',
  `status` enum('sps_play_subscribed','sps_play_invited','sps_edit_invited','sps_anim_invited','sps_playing','sps_editing','sps_animating') NOT NULL DEFAULT 'sps_play_subscribed',
  `kicked` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `session_rated` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  KEY `session_idx` (`session_id`),
  KEY `user_idx` (`char_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

# Data exporting was unselected.


# Dumping structure for table ring_open.shard
CREATE TABLE IF NOT EXISTS `shard` (
  `shard_id` int(10) NOT NULL DEFAULT '0',
  `WSOnline` tinyint(1) NOT NULL DEFAULT '0',
  `MOTD` text NOT NULL,
  `OldState` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_restricted',
  `RequiredState` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev',
  PRIMARY KEY (`shard_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

# Data exporting was unselected.
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
