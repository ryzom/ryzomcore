-- phpMyAdmin SQL Dump
-- version 3.3.3
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Erstellungszeit: 27. Mai 2012 um 21:06
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

--
-- Daten für Tabelle `ach_achievement`
--

INSERT INTO `ach_achievement` (`aa_id`, `aa_category`, `aa_parent`, `aa_tie_race`, `aa_tie_cult`, `aa_tie_civ`, `aa_image`) VALUES
(1, 1, NULL, NULL, NULL, NULL, ''),
(2, 1, NULL, NULL, NULL, NULL, ''),
(3, 1, NULL, NULL, NULL, NULL, ''),
(4, 1, NULL, NULL, NULL, NULL, '');

--
-- Daten für Tabelle `ach_achievement_lang`
--

INSERT INTO `ach_achievement_lang` (`aal_achievement`, `aal_lang`, `aal_name`) VALUES
(1, 'en', 'Kill the Bait'),
(2, 'en', 'Grill the Bill'),
(3, 'en', 'Killing Spree'),
(4, 'en', 'The Burning Desert');

--
-- Daten für Tabelle `ach_atom`
--

INSERT INTO `ach_atom` (`atom_id`, `atom_objective`, `atom_mandatory`, `atom_ruleset`, `atom_primary`) VALUES
(1, 13, 0, '', 0);

--
-- Daten für Tabelle `ach_category`
--

INSERT INTO `ach_category` (`ac_id`, `ac_parent`, `ac_order`, `ac_image`) VALUES
(1, NULL, 0, ''),
(2, NULL, 0, ''),
(3, 1, 0, ''),
(4, 1, 0, ''),
(5, 1, 0, '');

--
-- Daten für Tabelle `ach_category_lang`
--

INSERT INTO `ach_category_lang` (`acl_category`, `acl_lang`, `acl_name`) VALUES
(1, 'en', 'Exploration'),
(2, 'en', 'Occupations'),
(3, 'en', 'Aeden Aqueous'),
(4, 'en', 'Burning Desert'),
(5, 'en', 'Witherings');

--
-- Daten für Tabelle `ach_objective`
--

INSERT INTO `ach_objective` (`ao_id`, `ao_perk`, `ao_condition`, `ao_value`, `ao_display`) VALUES
(1, 1, 'all', NULL, 'simple'),
(2, 1, 'all', NULL, 'simple'),
(3, 1, 'all', NULL, 'simple'),
(4, 1, 'all', NULL, 'simple'),
(5, 1, 'all', NULL, 'simple'),
(6, 2, 'all', NULL, 'hidden'),
(7, 3, 'all', 30, 'value'),
(8, 4, 'all', NULL, 'meta'),
(9, 4, 'all', NULL, 'meta'),
(10, 4, 'all', NULL, 'meta'),
(11, 4, 'all', NULL, 'meta'),
(12, 4, 'all', NULL, 'meta'),
(13, 3, 'all', 30, 'value');

--
-- Daten für Tabelle `ach_objective_lang`
--

INSERT INTO `ach_objective_lang` (`aol_objective`, `aol_lang`, `aol_name`) VALUES
(1, 'en', 'Kill A'),
(2, 'en', 'Kill B'),
(3, 'en', 'Kill C'),
(4, 'en', 'Kill D'),
(5, 'en', 'Kill E'),
(7, 'en', 'Kill 30 random Yubos'),
(8, 'en', 'Meta A'),
(9, 'en', 'Meta B'),
(10, 'en', 'Meta C'),
(11, 'en', 'Meta D'),
(12, 'en', 'Meta E'),
(13, 'en', 'Kill 30 random Gingos');

--
-- Daten für Tabelle `ach_perk`
--

INSERT INTO `ach_perk` (`ap_id`, `ap_achievement`, `ap_parent`, `ap_value`) VALUES
(1, 1, NULL, 50),
(2, 2, NULL, 10),
(3, 3, NULL, 10),
(4, 4, NULL, 20),
(5, 2, NULL, 10),
(6, 2, NULL, 10);

--
-- Daten für Tabelle `ach_perk_lang`
--

INSERT INTO `ach_perk_lang` (`apl_perk`, `apl_lang`, `apl_name`) VALUES
(1, 'en', 'Murder every boss listed below'),
(2, 'en', 'Grill "Bill the Vile"'),
(3, 'en', 'Kill 30 of each mob type listed below'),
(4, 'en', 'Explore all regions of the Burning Desert'),
(5, 'en', 'Grill "Peter the Pan"'),
(6, 'en', 'Grill "Ivan the Slayer"');

--
-- Daten für Tabelle `ach_player_atom`
--

INSERT INTO `ach_player_atom` (`apa_atom`, `apa_player`, `apa_date`, `apa_expire`) VALUES
(1, 1, 0, '');

--
-- Daten für Tabelle `ach_player_objective`
--

INSERT INTO `ach_player_objective` (`apo_objective`, `apo_player`, `apo_date`) VALUES
(4, 1, 500),
(7, 1, 500),
(11, 1, 500);

--
-- Daten für Tabelle `ach_player_perk`
--

INSERT INTO `ach_player_perk` (`app_perk`, `app_player`, `app_date`) VALUES
(2, 1, 600),
(5, 1, 100);

--
-- Daten für Tabelle `ach_player_valuecache`
--

