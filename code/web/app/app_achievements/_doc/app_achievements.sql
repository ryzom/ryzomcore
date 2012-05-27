-- phpMyAdmin SQL Dump
-- version 3.3.3
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Erstellungszeit: 27. Mai 2012 um 21:05
-- Server Version: 5.1.46
-- PHP-Version: 5.3.2

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Datenbank: `app_achievements`
--

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_achievement`
--

CREATE TABLE IF NOT EXISTS `ach_achievement` (
  `aa_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `aa_category` bigint(20) unsigned NOT NULL,
  `aa_parent` bigint(20) unsigned DEFAULT NULL,
  `aa_tie_race` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `aa_tie_cult` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `aa_tie_civ` varchar(64) COLLATE utf8_bin DEFAULT NULL,
  `aa_image` varchar(64) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`aa_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=5 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_achievement_lang`
--

CREATE TABLE IF NOT EXISTS `ach_achievement_lang` (
  `aal_achievement` bigint(20) unsigned NOT NULL,
  `aal_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aal_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`aal_achievement`,`aal_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_atom`
--

CREATE TABLE IF NOT EXISTS `ach_atom` (
  `atom_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `atom_objective` bigint(20) unsigned NOT NULL,
  `atom_mandatory` tinyint(1) unsigned NOT NULL,
  `atom_ruleset` blob NOT NULL,
  `atom_primary` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`atom_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=2 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_category`
--

CREATE TABLE IF NOT EXISTS `ach_category` (
  `ac_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ac_parent` bigint(20) unsigned DEFAULT NULL,
  `ac_order` smallint(5) unsigned NOT NULL,
  `ac_image` varchar(64) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`ac_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=6 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_category_lang`
--

CREATE TABLE IF NOT EXISTS `ach_category_lang` (
  `acl_category` bigint(20) unsigned NOT NULL,
  `acl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `acl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`acl_category`,`acl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_objective`
--

CREATE TABLE IF NOT EXISTS `ach_objective` (
  `ao_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ao_perk` bigint(20) unsigned NOT NULL,
  `ao_condition` enum('all','any','value') COLLATE utf8_bin NOT NULL,
  `ao_value` int(10) unsigned DEFAULT NULL,
  `ao_display` enum('simple','meta','value','hidden') COLLATE utf8_bin NOT NULL DEFAULT 'hidden',
  PRIMARY KEY (`ao_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=14 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_objective_lang`
--

CREATE TABLE IF NOT EXISTS `ach_objective_lang` (
  `aol_objective` bigint(20) unsigned NOT NULL,
  `aol_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `aol_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`aol_objective`,`aol_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_perk`
--

CREATE TABLE IF NOT EXISTS `ach_perk` (
  `ap_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ap_achievement` bigint(20) unsigned NOT NULL,
  `ap_parent` bigint(20) unsigned DEFAULT NULL,
  `ap_value` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ap_id`),
  UNIQUE KEY `ap_parent` (`ap_parent`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=7 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_perk_lang`
--

CREATE TABLE IF NOT EXISTS `ach_perk_lang` (
  `apl_perk` bigint(20) unsigned NOT NULL,
  `apl_lang` varchar(2) COLLATE utf8_bin NOT NULL,
  `apl_name` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`apl_perk`,`apl_lang`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_player_atom`
--

CREATE TABLE IF NOT EXISTS `ach_player_atom` (
  `apa_atom` bigint(20) unsigned NOT NULL,
  `apa_player` bigint(20) unsigned NOT NULL,
  `apa_date` bigint(20) unsigned NOT NULL,
  `apa_expire` blob NOT NULL,
  KEY `apa_atom` (`apa_atom`,`apa_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_player_objective`
--

CREATE TABLE IF NOT EXISTS `ach_player_objective` (
  `apo_objective` bigint(20) unsigned NOT NULL,
  `apo_player` bigint(20) unsigned NOT NULL,
  `apo_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apo_objective`,`apo_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_player_perk`
--

CREATE TABLE IF NOT EXISTS `ach_player_perk` (
  `app_perk` bigint(20) unsigned NOT NULL,
  `app_player` bigint(20) unsigned NOT NULL,
  `app_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`app_perk`,`app_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_player_valuecache`
--

CREATE TABLE IF NOT EXISTS `ach_player_valuecache` (
  `apv_name` bigint(20) unsigned NOT NULL,
  `apv_player` bigint(20) unsigned NOT NULL,
  `apv_value` varchar(255) COLLATE utf8_bin NOT NULL,
  `apv_date` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`apv_name`,`apv_player`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
