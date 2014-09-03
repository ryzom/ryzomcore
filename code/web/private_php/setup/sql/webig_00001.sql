-- --------------------------------------------------------
-- Host:                         178.33.225.92
-- Server version:               5.5.28-0ubuntu0.12.04.2-log - (Ubuntu)
-- Server OS:                    debian-linux-gnu
-- HeidiSQL version:             7.0.0.4053
-- Date/time:                    2013-02-12 16:14:54
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;

-- Dumping database structure for webig
-- CREATE DATABASE IF NOT EXISTS `webig` /*!40100 DEFAULT CHARACTER SET utf8 COLLATE utf8_bin */;
-- USE `webig`;


-- Dumping structure for table webig.players
CREATE TABLE IF NOT EXISTS `players` (
  `id` INT(32) NOT NULL AUTO_INCREMENT,
  `cid` INT(32) NOT NULL DEFAULT '0',
  `name` VARCHAR(50) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '0',
  `gender` INT(1) NOT NULL DEFAULT '0',
  `creation_date` TIMESTAMP NOT NULL DEFAULT '0000-00-00 00:00:00',
  `deleted` tinyint(1) NOT NULL DEFAULT '0',
  `last_login` TIMESTAMP NOT NULL DEFAULT '0000-00-00 00:00:00',
  `dev_shard` tinyint(4) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Dumping structure for table webig.accounts
CREATE TABLE IF NOT EXISTS `accounts` (
  `uid` INT(10) DEFAULT NULL,
  `web_privs` VARCHAR(255) COLLATE utf8_bin DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- Data exporting was unselected.
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

