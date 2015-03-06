-- --------------------------------------------------------
-- Host:                         178.33.225.92
-- Server version:               5.5.28-0ubuntu0.12.04.2-log - (Ubuntu)
-- Server OS:                    debian-linux-gnu
-- HeidiSQL version:             7.0.0.4053
-- Date/time:                    2013-02-12 16:31:42
-- --------------------------------------------------------
 
/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET FOREIGN_KEY_CHECKS=0 */;
 
-- Dumping database structure for app_test
CREATE DATABASE IF NOT EXISTS `app_test` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `app_test`;
 
 
-- Dumping structure for table app_test.test
CREATE TABLE IF NOT EXISTS `test` (
  `id` INT(32) NOT NULL,
  `num_access` INT(10) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
 
-- Data exporting was unselected.
/*!40014 SET FOREIGN_KEY_CHECKS=1 */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;

