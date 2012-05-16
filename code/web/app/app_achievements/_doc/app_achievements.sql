-- phpMyAdmin SQL Dump
-- version 3.3.3
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Erstellungszeit: 17. Mai 2012 um 01:02
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
  `aa_image` varchar(255) COLLATE utf8_bin NOT NULL,
  PRIMARY KEY (`aa_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1 ;

--
-- Daten für Tabelle `ach_achievement`
--


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

--
-- Daten für Tabelle `ach_achievement_lang`
--


-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_atom`
--

CREATE TABLE IF NOT EXISTS `ach_atom` (
  `atom_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `atom_objective` bigint(20) unsigned NOT NULL,
  `atom_mandatory` tinyint(1) unsigned NOT NULL,
  `atom_ruleset` blob NOT NULL,
  PRIMARY KEY (`atom_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1 ;

--
-- Daten für Tabelle `ach_atom`
--


-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_category`
--

CREATE TABLE IF NOT EXISTS `ach_category` (
  `ac_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ac_parent` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`ac_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1 ;

--
-- Daten für Tabelle `ach_category`
--


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

--
-- Daten für Tabelle `ach_category_lang`
--


-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ach_objective`
--

CREATE TABLE IF NOT EXISTS `ach_objective` (
  `ao_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ao_perk` bigint(20) unsigned NOT NULL,
  `ao_condition` enum('all','any','value') COLLATE utf8_bin NOT NULL,
  `ao_value` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`ao_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1 ;

--
-- Daten für Tabelle `ach_objective`
--


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

--
-- Daten für Tabelle `ach_objective_lang`
--


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
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin AUTO_INCREMENT=1 ;

--
-- Daten für Tabelle `ach_perk`
--


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

--
-- Daten für Tabelle `ach_perk_lang`
--

