-- phpMyAdmin SQL Dump
-- version 3.4.10.1deb1
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 14, 2014 at 09:58 AM
-- Server version: 5.5.37
-- PHP Version: 5.3.10-1ubuntu3.11

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `nel`
--

-- --------------------------------------------------------

--
-- Table structure for table `domain`
--

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=21 ;

-- --------------------------------------------------------

--
-- Table structure for table `permission`
--

CREATE TABLE IF NOT EXISTS `permission` (
  `UId` int(10) unsigned NOT NULL DEFAULT '0',
  `ClientApplication` char(64) NOT NULL DEFAULT 'ryzom',
  `ShardId` int(10) NOT NULL DEFAULT '-1',
  `AccessPrivilege` set('OPEN','DEV','RESTRICTED') NOT NULL DEFAULT 'OPEN',
  `prim` int(10) unsigned NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`prim`),
  KEY `UIDIndex` (`UId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=8 ;

-- --------------------------------------------------------

--
-- Table structure for table `shard`
--

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COMMENT='contains all shards information for login system' AUTO_INCREMENT=31 ;

-- --------------------------------------------------------

--
-- Table structure for table `user`
--

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COMMENT='contains all users information for login system' AUTO_INCREMENT=5 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
