-- phpMyAdmin SQL Dump
-- version 3.5.1
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Aug 20, 2012 at 01:48 PM
-- Server version: 5.5.24-log
-- PHP Version: 5.4.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `app_achievements_test`
--

-- --------------------------------------------------------

--
-- Table structure for table `ach_achievement`
--

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=346 ;

-- --------------------------------------------------------

--
-- Table structure for table `ach_achievement_lang`
--

CREATE TABLE IF NOT EXISTS `ach_achievement_lang` (
  `aal_achievement` bigint(20) unsigned NOT NULL,
  `aal_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aal_name` varchar(255) COLLATE utf8_bin NOT NULL,
  `aal_template` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`aal_achievement`,`aal_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_atom`
--

CREATE TABLE IF NOT EXISTS `ach_atom` (
  `atom_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `atom_objective` bigint(20) unsigned NOT NULL,
  `atom_mandatory` tinyint(1) unsigned NOT NULL,
  `atom_ruleset` blob NOT NULL,
  `atom_ruleset_parsed` blob NOT NULL,
  PRIMARY KEY (`atom_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=25 ;

-- --------------------------------------------------------

--
-- Table structure for table `ach_category`
--

CREATE TABLE IF NOT EXISTS `ach_category` (
  `ac_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ac_parent` bigint(20) unsigned DEFAULT NULL,
  `ac_order` smallint(5) unsigned NOT NULL,
  `ac_image` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `ac_dev` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `ac_heroic` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`ac_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=50 ;

-- --------------------------------------------------------

--
-- Table structure for table `ach_category_lang`
--

CREATE TABLE IF NOT EXISTS `ach_category_lang` (
  `acl_category` bigint(20) unsigned NOT NULL,
  `acl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `acl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`acl_category`,`acl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_fb_token`
--

CREATE TABLE IF NOT EXISTS `ach_fb_token` (
  `aft_player` bigint(20) unsigned NOT NULL,
  `aft_token` varchar(255) NOT NULL,
  `aft_date` bigint(20) unsigned NOT NULL,
  `aft_allow` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`aft_player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `ach_objective`
--

CREATE TABLE IF NOT EXISTS `ach_objective` (
  `ao_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ao_task` bigint(20) unsigned NOT NULL,
  `ao_condition` enum('all','any','value') COLLATE utf8_bin NOT NULL,
  `ao_value` bigint(20) unsigned DEFAULT NULL,
  `ao_display` enum('simple','meta','value','hidden') COLLATE utf8_bin NOT NULL DEFAULT 'hidden',
  `ao_metalink` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`ao_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=2026 ;

-- --------------------------------------------------------

--
-- Table structure for table `ach_objective_lang`
--

CREATE TABLE IF NOT EXISTS `ach_objective_lang` (
  `aol_objective` bigint(20) unsigned NOT NULL,
  `aol_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aol_name` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`aol_objective`,`aol_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_player_atom`
--

CREATE TABLE IF NOT EXISTS `ach_player_atom` (
  `apa_atom` bigint(20) unsigned NOT NULL,
  `apa_player` bigint(20) unsigned NOT NULL,
  `apa_date` bigint(20) unsigned NOT NULL,
  `apa_expire` blob,
  `apa_state` enum('GRANT','DENY') COLLATE utf8_bin NOT NULL,
  `apa_value` bigint(20) unsigned NOT NULL,
  KEY `apa_atom` (`apa_atom`,`apa_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_player_objective`
--

CREATE TABLE IF NOT EXISTS `ach_player_objective` (
  `apo_objective` bigint(20) unsigned NOT NULL,
  `apo_player` bigint(20) unsigned NOT NULL,
  `apo_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apo_objective`,`apo_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_player_task`
--

CREATE TABLE IF NOT EXISTS `ach_player_task` (
  `apt_task` bigint(20) unsigned NOT NULL,
  `apt_player` bigint(20) unsigned NOT NULL,
  `apt_date` bigint(20) unsigned NOT NULL,
  `apt_fb` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`apt_task`,`apt_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_player_valuecache`
--

CREATE TABLE IF NOT EXISTS `ach_player_valuecache` (
  `apv_name` bigint(20) unsigned NOT NULL,
  `apv_player` bigint(20) unsigned NOT NULL,
  `apv_value` varchar(255) COLLATE utf8_bin NOT NULL,
  `apv_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apv_name`,`apv_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `ach_task`
--

CREATE TABLE IF NOT EXISTS `ach_task` (
  `at_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `at_achievement` bigint(20) unsigned NOT NULL,
  `at_parent` bigint(20) unsigned DEFAULT NULL,
  `at_value` int(10) unsigned NOT NULL,
  `at_condition` enum('all','any','value') COLLATE utf8_bin NOT NULL DEFAULT 'all',
  `at_condition_value` int(10) unsigned DEFAULT NULL,
  `at_dev` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `at_torder` smallint(5) unsigned NOT NULL,
  PRIMARY KEY (`at_id`),
  UNIQUE KEY `ap_parent` (`at_parent`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=635 ;

-- --------------------------------------------------------

--
-- Table structure for table `ach_task_lang`
--

CREATE TABLE IF NOT EXISTS `ach_task_lang` (
  `atl_task` bigint(20) unsigned NOT NULL,
  `atl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `atl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  `atl_template` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`atl_task`,`atl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
