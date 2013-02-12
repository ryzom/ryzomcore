-- --------------------------------------------------------
-- Host:                         178.33.225.92
-- Server version:               5.5.28-0ubuntu0.12.04.2-log - (Ubuntu)
-- Server OS:                    debian-linux-gnu
-- HeidiSQL version:             7.0.0.4053
-- Date/time:                    2012-12-10 14:52:03
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

-- Dumping structure for table app_achievements.ach_achievement
CREATE TABLE IF NOT EXISTS `ach_achievement` (
  `aa_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `aa_category` bigint(20) unsigned NOT NULL,
  `aa_parent` bigint(20) unsigned DEFAULT NULL,
  `aa_tie_race` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `aa_tie_cult` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `aa_tie_civ` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `aa_image` varchar(64) COLLATE utf8_bin NOT NULL,
  `aa_dev` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `aa_sticky` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`aa_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_achievement_lang
CREATE TABLE IF NOT EXISTS `ach_achievement_lang` (
  `aal_achievement` bigint(20) unsigned NOT NULL,
  `aal_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aal_name` varchar(255) COLLATE utf8_bin NOT NULL,
  `aal_template` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`aal_achievement`,`aal_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_atom
CREATE TABLE IF NOT EXISTS `ach_atom` (
  `atom_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `atom_objective` bigint(20) unsigned NOT NULL,
  `atom_mandatory` tinyint(1) unsigned NOT NULL,
  `atom_ruleset` blob NOT NULL,
  `atom_ruleset_parsed` blob NOT NULL,
  PRIMARY KEY (`atom_id`),
  KEY `atom_objective` (`atom_objective`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_category
CREATE TABLE IF NOT EXISTS `ach_category` (
  `ac_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ac_parent` bigint(20) unsigned DEFAULT NULL,
  `ac_order` smallint(5) unsigned NOT NULL,
  `ac_image` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `ac_dev` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `ac_heroic` tinyint(1) unsigned NOT NULL,
  `ac_contest` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`ac_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_category_lang
CREATE TABLE IF NOT EXISTS `ach_category_lang` (
  `acl_category` bigint(20) unsigned NOT NULL,
  `acl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `acl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`acl_category`,`acl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_fb_token
CREATE TABLE IF NOT EXISTS `ach_fb_token` (
  `aft_player` bigint(20) unsigned NOT NULL,
  `aft_token` varchar(255) NOT NULL,
  `aft_date` bigint(20) unsigned NOT NULL,
  `aft_allow` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`aft_player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_objective
CREATE TABLE IF NOT EXISTS `ach_objective` (
  `ao_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ao_task` bigint(20) unsigned NOT NULL,
  `ao_condition` enum('all','any','value') COLLATE utf8_bin NOT NULL,
  `ao_value` bigint(20) unsigned DEFAULT NULL,
  `ao_display` enum('simple','meta','value','hidden') COLLATE utf8_bin NOT NULL DEFAULT 'hidden',
  `ao_metalink` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`ao_id`),
  KEY `ao_task` (`ao_task`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_objective_lang
CREATE TABLE IF NOT EXISTS `ach_objective_lang` (
  `aol_objective` bigint(20) unsigned NOT NULL,
  `aol_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aol_name` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`aol_objective`,`aol_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_player_atom
CREATE TABLE IF NOT EXISTS `ach_player_atom` (
  `apa_atom` bigint(20) unsigned NOT NULL,
  `apa_player` bigint(20) unsigned NOT NULL,
  `apa_date` bigint(20) unsigned NOT NULL,
  `apa_expire` blob,
  `apa_state` enum('GRANT','DENY') COLLATE utf8_bin NOT NULL,
  `apa_value` bigint(20) unsigned NOT NULL,
  KEY `apa_atom` (`apa_atom`,`apa_player`),
  KEY `apa_state` (`apa_state`),
  KEY `apa_atom_2` (`apa_atom`,`apa_player`,`apa_state`),
  KEY `apa_player` (`apa_player`),
  KEY `apa_atom_3` (`apa_atom`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_player_objective
CREATE TABLE IF NOT EXISTS `ach_player_objective` (
  `apo_objective` bigint(20) unsigned NOT NULL,
  `apo_player` bigint(20) unsigned NOT NULL,
  `apo_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apo_objective`,`apo_player`),
  KEY `apo_player` (`apo_player`),
  KEY `apo_objective` (`apo_objective`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_player_task
CREATE TABLE IF NOT EXISTS `ach_player_task` (
  `apt_task` bigint(20) unsigned NOT NULL,
  `apt_player` bigint(20) unsigned NOT NULL,
  `apt_date` bigint(20) unsigned NOT NULL,
  `apt_fb` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`apt_task`,`apt_player`),
  KEY `apt_player` (`apt_player`),
  KEY `apt_task` (`apt_task`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_player_valuecache
CREATE TABLE IF NOT EXISTS `ach_player_valuecache` (
  `apv_name` varchar(64) COLLATE utf8_bin NOT NULL,
  `apv_player` bigint(20) unsigned NOT NULL,
  `apv_value` varchar(255) COLLATE utf8_bin NOT NULL,
  `apv_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apv_name`,`apv_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_task
CREATE TABLE IF NOT EXISTS `ach_task` (
  `at_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `at_achievement` bigint(20) unsigned NOT NULL,
  `at_parent` bigint(20) unsigned DEFAULT NULL,
  `at_value` int(10) unsigned NOT NULL,
  `at_condition` enum('all','any','value') COLLATE utf8_bin NOT NULL DEFAULT 'all',
  `at_condition_value` int(10) unsigned DEFAULT NULL,
  `at_dev` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `at_torder` smallint(5) unsigned NOT NULL,
  `at_inherit` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`at_id`),
  UNIQUE KEY `ap_parent` (`at_parent`),
  KEY `at_achievement` (`at_achievement`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_task_lang
CREATE TABLE IF NOT EXISTS `ach_task_lang` (
  `atl_task` bigint(20) unsigned NOT NULL,
  `atl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `atl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  `atl_template` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`atl_task`,`atl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_task_tie_civ
CREATE TABLE IF NOT EXISTS `ach_task_tie_civ` (
  `attciv_task` bigint(20) unsigned NOT NULL,
  `attciv_civ` varchar(64) NOT NULL,
  PRIMARY KEY (`attciv_task`,`attciv_civ`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_task_tie_cult
CREATE TABLE IF NOT EXISTS `ach_task_tie_cult` (
  `attcult_task` bigint(20) unsigned NOT NULL,
  `attcult_cult` varchar(64) NOT NULL,
  PRIMARY KEY (`attcult_task`,`attcult_cult`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.ach_task_tie_race
CREATE TABLE IF NOT EXISTS `ach_task_tie_race` (
  `attr_task` bigint(20) unsigned NOT NULL,
  `attr_race` varchar(64) NOT NULL,
  PRIMARY KEY (`attr_task`,`attr_race`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.stat_daily
CREATE TABLE IF NOT EXISTS `stat_daily` (
  `sd_day` date NOT NULL DEFAULT '0000-00-00',
  `sd_players` bigint(20) unsigned DEFAULT NULL,
  `sd_money_total` bigint(20) unsigned DEFAULT NULL,
  `sd_money_avg` int(10) unsigned DEFAULT NULL,
  `sd_money_mean` int(10) unsigned DEFAULT NULL,
  `sd_mek_total` bigint(20) unsigned DEFAULT NULL,
  `sd_mek_avg` smallint(5) unsigned DEFAULT NULL,
  `sd_mek_mean` smallint(5) unsigned DEFAULT NULL,
  `sd_lvl_total` bigint(20) unsigned DEFAULT NULL,
  `sd_lvl_avg` int(10) unsigned DEFAULT NULL,
  `sd_lvl_mean` int(10) unsigned DEFAULT NULL,
  `sd_item_total` bigint(20) unsigned DEFAULT NULL,
  `sd_item_avg` int(10) unsigned DEFAULT NULL,
  `sd_item_mean` int(10) unsigned DEFAULT NULL,
  `sd_yubo_total` bigint(20) unsigned DEFAULT NULL,
  `sd_yubo_avg` int(10) unsigned DEFAULT NULL,
  `sd_yubo_mean` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`sd_day`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.


-- Dumping structure for table app_achievements.stat_players
CREATE TABLE IF NOT EXISTS `stat_players` (
  `sp_char` bigint(20) unsigned NOT NULL DEFAULT '0',
  `sp_money` bigint(20) unsigned DEFAULT NULL,
  `sp_race` enum('r_matis','r_tryker','r_fyros','r_zorai') DEFAULT NULL,
  `sp_yubototal` int(10) unsigned DEFAULT NULL,
  `sp_mekcount` int(10) unsigned DEFAULT NULL,
  `sp_maxlevel` smallint(5) unsigned DEFAULT NULL,
  `sp_guildid` int(10) unsigned DEFAULT NULL,
  `sp_itemcount` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`sp_char`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
