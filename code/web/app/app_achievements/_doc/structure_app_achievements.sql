-- --------------------------------------------------------
-- Host:                         178.33.225.92
-- Server Version:               5.5.28-0ubuntu0.12.04.2-log - (Ubuntu)
-- Server Betriebssystem:        debian-linux-gnu
-- HeidiSQL Version:             7.0.0.4328
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Exportiere Struktur von Tabelle app_achievements.ach_achievement
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

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_achievement_lang
CREATE TABLE IF NOT EXISTS `ach_achievement_lang` (
  `aal_achievement` bigint(20) unsigned NOT NULL,
  `aal_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aal_name` varchar(255) COLLATE utf8_bin NOT NULL,
  `aal_template` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`aal_achievement`,`aal_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_atom
CREATE TABLE IF NOT EXISTS `ach_atom` (
  `atom_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `atom_objective` bigint(20) unsigned NOT NULL,
  `atom_mandatory` tinyint(1) unsigned NOT NULL,
  `atom_ruleset` blob NOT NULL,
  `atom_ruleset_parsed` blob NOT NULL,
  PRIMARY KEY (`atom_id`),
  KEY `atom_objective` (`atom_objective`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_category
CREATE TABLE IF NOT EXISTS `ach_category` (
  `ac_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ac_parent` bigint(20) unsigned DEFAULT NULL,
  `ac_order` smallint(5) unsigned NOT NULL,
  `ac_image` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `ac_dev` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `ac_heroic` tinyint(1) unsigned NOT NULL,
  `ac_contest` tinyint(1) unsigned NOT NULL,
  `ac_allow_civ` tinyint(1) unsigned NOT NULL,
  `ac_allow_cult` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`ac_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_category_lang
CREATE TABLE IF NOT EXISTS `ach_category_lang` (
  `acl_category` bigint(20) unsigned NOT NULL,
  `acl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `acl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`acl_category`,`acl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_fb_token
CREATE TABLE IF NOT EXISTS `ach_fb_token` (
  `aft_player` bigint(20) unsigned NOT NULL,
  `aft_token` varchar(255) NOT NULL,
  `aft_date` bigint(20) unsigned NOT NULL,
  `aft_allow` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`aft_player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_objective
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

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_objective_lang
CREATE TABLE IF NOT EXISTS `ach_objective_lang` (
  `aol_objective` bigint(20) unsigned NOT NULL,
  `aol_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aol_name` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`aol_objective`,`aol_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_player_atom
CREATE TABLE IF NOT EXISTS `ach_player_atom` (
  `apa_atom` bigint(10) unsigned NOT NULL,
  `apa_player` bigint(10) unsigned NOT NULL,
  `apa_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `apa_date` bigint(20) unsigned NOT NULL,
  `apa_expire` blob,
  `apa_state` enum('GRANT','DENY') COLLATE utf8_bin NOT NULL,
  `apa_value` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apa_id`),
  KEY `apa_atom` (`apa_atom`,`apa_player`),
  KEY `apa_atom_2` (`apa_atom`,`apa_player`,`apa_state`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin ROW_FORMAT=FIXED;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_player_item
CREATE TABLE IF NOT EXISTS `ach_player_item` (
  `api_item` varchar(32) COLLATE utf8_bin NOT NULL,
  `api_player` int(10) unsigned NOT NULL,
  `api_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`api_item`,`api_player`),
  KEY `Index 2` (`api_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin DELAY_KEY_WRITE=1 ROW_FORMAT=FIXED;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_player_objective
CREATE TABLE IF NOT EXISTS `ach_player_objective` (
  `apo_objective` bigint(20) unsigned NOT NULL,
  `apo_player` bigint(20) unsigned NOT NULL,
  `apo_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apo_objective`,`apo_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin DELAY_KEY_WRITE=1 ROW_FORMAT=FIXED;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_player_task
CREATE TABLE IF NOT EXISTS `ach_player_task` (
  `apt_task` bigint(20) unsigned NOT NULL,
  `apt_player` bigint(20) unsigned NOT NULL,
  `apt_date` bigint(20) unsigned NOT NULL,
  `apt_fb` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`apt_task`,`apt_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin DELAY_KEY_WRITE=1;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_player_valuecache
CREATE TABLE IF NOT EXISTS `ach_player_valuecache` (
  `apv_name` varchar(10) COLLATE utf8_bin NOT NULL,
  `apv_player` bigint(10) unsigned NOT NULL,
  `apv_value` varchar(255) COLLATE utf8_bin NOT NULL,
  `apv_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `apv_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apv_id`),
  UNIQUE KEY `key1` (`apv_name`,`apv_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin DELAY_KEY_WRITE=1 ROW_FORMAT=FIXED;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_task
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

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_task_lang
CREATE TABLE IF NOT EXISTS `ach_task_lang` (
  `atl_task` bigint(20) unsigned NOT NULL,
  `atl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `atl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  `atl_template` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`atl_task`,`atl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_task_tie_align
CREATE TABLE IF NOT EXISTS `ach_task_tie_align` (
  `atta_task` bigint(20) unsigned NOT NULL DEFAULT '0',
  `atta_alignment` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`atta_task`,`atta_alignment`),
  KEY `Index 2` (`atta_task`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_task_tie_civ
CREATE TABLE IF NOT EXISTS `ach_task_tie_civ` (
  `attciv_task` int(10) DEFAULT NULL,
  `attciv_civ` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_task_tie_cult
CREATE TABLE IF NOT EXISTS `ach_task_tie_cult` (
  `attcult_cult` int(10) DEFAULT NULL,
  `attcult_task` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ach_task_tie_race
CREATE TABLE IF NOT EXISTS `ach_task_tie_race` (
  `attr_task` bigint(20) unsigned NOT NULL,
  `attr_race` varchar(64) NOT NULL,
  PRIMARY KEY (`attr_task`,`attr_race`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ryzom_nimetu_item_data
CREATE TABLE IF NOT EXISTS `ryzom_nimetu_item_data` (
  `sheetid` varchar(64) COLLATE utf8_bin NOT NULL,
  `type` int(10) unsigned NOT NULL,
  `item_type` int(10) unsigned NOT NULL,
  `race` int(10) unsigned NOT NULL,
  `quality` int(10) unsigned NOT NULL,
  `craftplan` varchar(32) COLLATE utf8_bin NOT NULL,
  `skill` varchar(16) COLLATE utf8_bin NOT NULL,
  `damage` int(10) unsigned NOT NULL,
  `reach` int(10) unsigned NOT NULL,
  `ecosystem` int(10) unsigned NOT NULL,
  `grade` int(10) unsigned NOT NULL,
  `mpft` bigint(20) unsigned NOT NULL,
  `color` int(10) unsigned NOT NULL,
  `is_looted` int(10) unsigned NOT NULL,
  `is_mission` int(10) unsigned NOT NULL,
  `index` int(10) unsigned NOT NULL,
  `txt` text COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`sheetid`),
  KEY `type` (`type`),
  KEY `item_type` (`item_type`),
  KEY `type_2` (`type`,`item_type`),
  KEY `race` (`race`),
  KEY `quality` (`quality`),
  KEY `craftplan` (`craftplan`),
  KEY `skill` (`skill`),
  KEY `damage` (`damage`),
  KEY `reach` (`reach`),
  KEY `ecosystem` (`ecosystem`),
  KEY `grade` (`grade`),
  KEY `mpft` (`mpft`),
  KEY `color` (`color`),
  KEY `is_looted` (`is_looted`),
  KEY `is_mission` (`is_mission`),
  KEY `index` (`index`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ryzom_nimetu_sheets
CREATE TABLE IF NOT EXISTS `ryzom_nimetu_sheets` (
  `nsh_numid` bigint(20) NOT NULL,
  `nsh_name` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `nsh_suffix` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`nsh_numid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.ryzom_title
CREATE TABLE IF NOT EXISTS `ryzom_title` (
  `t_id` varchar(255) CHARACTER SET utf8 NOT NULL,
  `t_lang` varchar(2) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `t_male` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `t_female` varchar(255) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`t_id`,`t_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.stat_daily
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

-- Daten Export vom Benutzer nicht ausgewählt


-- Exportiere Struktur von Tabelle app_achievements.stat_players
CREATE TABLE IF NOT EXISTS `stat_players` (
  `sp_char` bigint(10) unsigned NOT NULL DEFAULT '0',
  `sp_money` bigint(20) unsigned DEFAULT NULL,
  `sp_race` enum('r_matis','r_tryker','r_fyros','r_zorai') DEFAULT NULL,
  `sp_yubototal` int(10) unsigned DEFAULT NULL,
  `sp_mekcount` int(10) unsigned DEFAULT NULL,
  `sp_maxlevel` smallint(5) unsigned DEFAULT NULL,
  `sp_guildid` int(10) unsigned DEFAULT NULL,
  `sp_itemcount` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`sp_char`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 DELAY_KEY_WRITE=1;

-- Daten Export vom Benutzer nicht ausgewählt
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
