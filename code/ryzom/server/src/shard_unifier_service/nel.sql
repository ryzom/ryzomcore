# MySQL-Front Dump 2.4
#
# Host: localhost   Database: nel
#--------------------------------------------------------
# Server version 4.0.24_Debian-10sarge1-log

USE nel;


#
# Table structure for table 'domain'
#

CREATE TABLE `domain` (
  `domain_id` int(10) unsigned NOT NULL auto_increment,
  `domain_name` varchar(32) NOT NULL default '',
  `status` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL default 'ds_dev',
  `patch_version` int(10) unsigned NOT NULL default '0',
  `backup_patch_url` varchar(255) default NULL,
  `patch_urls` text,
  `login_address` varchar(255) NOT NULL default '',
  `session_manager_address` varchar(255) NOT NULL default '',
  `ring_db_name` varchar(255) NOT NULL default '',
  `web_host` varchar(255) NOT NULL default '',
  `web_host_php` varchar(255) NOT NULL default '',
  `description` varchar(200) default NULL,
  PRIMARY KEY  (`domain_id`),
  UNIQUE KEY `name_idx` (`domain_name`)
) TYPE=MyISAM;



#
# Table structure for table 'permission'
#

CREATE TABLE `permission` (
  `UId` int(10) unsigned NOT NULL default '0',
  `ClientApplication` char(64) NOT NULL default 'r2',
  `ShardId` int(10) NOT NULL default '-1',
  `AccessPrivilege` set('DEV','RESTRICTED','OPEN') NOT NULL default 'OPEN',
  `prim` int(10) unsigned NOT NULL auto_increment,
  PRIMARY KEY  (`prim`),
  KEY `UIdIndex` (`UId`)
) TYPE=MyISAM;



#
# Table structure for table 'shard'
#

CREATE TABLE `shard` (
  `ShardId` int(10) NOT NULL auto_increment,
  `domain_id` int(10) NOT NULL default '-1',
  `WsAddr` varchar(64) default NULL,
  `NbPlayers` int(10) unsigned default '0',
  `Name` varchar(64) default 'unknown shard',
  `Online` tinyint(1) unsigned default '0',
  `ClientApplication` varchar(64) default 'ryzom',
  `Version` varchar(64) default NULL,
  `PatchURL` varchar(255) default NULL,
  `DynPatchURL` varchar(255) default NULL,
  `FixedSessionId` int(10) unsigned default '0',
  PRIMARY KEY  (`ShardId`)
) TYPE=MyISAM COMMENT='contains all shards information for login system';



#
# Table structure for table 'user'
#

CREATE TABLE `user` (
  `UId` int(10) NOT NULL auto_increment,
  `Login` varchar(64) NOT NULL default '',
  `Password` varchar(13) default NULL,
  `ShardId` int(10) NOT NULL default '-1',
  `State` enum('Offline','Online') NOT NULL default 'Offline',
  `Privilege` varchar(255) default NULL,
  `GroupName` varchar(255) NOT NULL default '',
  `FirstName` varchar(255) NOT NULL default '',
  `LastName` varchar(255) NOT NULL default '',
  `Birthday` varchar(32) NOT NULL default '',
  `Gender` tinyint(1) unsigned NOT NULL default '0',
  `Country` char(2) NOT NULL default '',
  `Email` varchar(255) NOT NULL default '',
  `Address` varchar(255) NOT NULL default '',
  `City` varchar(100) NOT NULL default '',
  `PostalCode` varchar(10) NOT NULL default '',
  `USState` char(2) NOT NULL default '',
  `Chat` char(2) NOT NULL default '0',
  `BetaKeyId` int(10) unsigned NOT NULL default '0',
  `CachedCoupons` varchar(255) NOT NULL default '',
  `ProfileAccess` varchar(45) default NULL,
  `Level` int(2) NOT NULL default '0',
  `CurrentFunds` int(4) NOT NULL default '0',
  `IdBilling` varchar(255) NOT NULL default '',
  `Community` char(2) NOT NULL default '--',
  `Newsletter` tinyint(1) NOT NULL default '1',
  `Account` varchar(64) NOT NULL default '',
  `ChoiceSubLength` tinyint(2) NOT NULL default '0',
  `CurrentSubLength` varchar(255) NOT NULL default '0',
  `ValidIdBilling` int(4) NOT NULL default '0',
  `GMId` int(4) NOT NULL default '0',
  `ExtendedPrivilege` varchar(255) NOT NULL default '',
  `ToolsGroup` varchar(255) NOT NULL default '',
  `Unsubscribe` date NOT NULL default '0000-00-00',
  `SubDate` datetime NOT NULL default '0000-00-00 00:00:00',
  `SubIp` varchar(20) NOT NULL default '',
  `SecurePassword` varchar(32) NOT NULL default '',
  `LastInvoiceEmailCheck` date NOT NULL default '0000-00-00',
  `FromSource` varchar(8) NOT NULL default '',
  `ValidMerchantCode` varchar(11) NOT NULL default '',
  `PBC` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`UId`),
  KEY `LoginIndex` (`Login`),
  KEY `GroupIndex` (`GroupName`),
  KEY `Email` (`Email`),
  KEY `ToolsGroup` (`ToolsGroup`),
  KEY `CurrentSubLength` (`CurrentSubLength`),
  KEY `Community` (`Community`),
  KEY `GMId` (`GMId`)
) TYPE=InnoDB;

