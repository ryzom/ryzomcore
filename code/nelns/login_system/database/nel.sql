# HeidiSQL Dump 
#
# --------------------------------------------------------
# Host:                 127.0.0.1
# Database:             nel
# Server version:       5.0.33
# Server OS:            Win32
# Target-Compatibility: MySQL 5.0
# Extended INSERTs:     Y
# max_allowed_packet:   1048576
# HeidiSQL version:     3.0 Revision: 572
# --------------------------------------------------------

/*!40100 SET CHARACTER SET latin1*/;


#
# Table structure for table 'permission'
#

CREATE TABLE `permission` (
  `UId` int(10) unsigned NOT NULL default '0',
  `ClientApplication` char(64) collate latin1_general_ci NOT NULL default 'sample',
  `ShardId` int(10) NOT NULL default '-1'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;



#
# Table structure for table 'shard'
#

CREATE TABLE `shard` (
  `ShardId` int(10) NOT NULL auto_increment,
  `WsAddr` varchar(64) collate latin1_general_ci NOT NULL,
  `NbPlayers` int(10) unsigned NOT NULL default '0',
  `Name` varchar(64) collate latin1_general_ci NOT NULL default 'unknown shard',
  `Online` tinyint(1) unsigned NOT NULL default '0',
  `ClientApplication` varchar(64) collate latin1_general_ci NOT NULL,
  `Version` varchar(64) collate latin1_general_ci NOT NULL default '',
  `DynPatchURL` varchar(255) collate latin1_general_ci NOT NULL default '',
  PRIMARY KEY  (`ShardId`)
) ENGINE=MyISAM AUTO_INCREMENT=301 DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='contains all shards information for login system';



#
# Table structure for table 'user'
#

CREATE TABLE `user` (
  `UId` int(10) NOT NULL auto_increment,
  `Login` varchar(64) collate latin1_general_ci NOT NULL default '',
  `Password` char(32) collate latin1_general_ci NOT NULL,
  `ShardId` int(10) NOT NULL default '-1',
  `State` enum('Offline','Authorized','Waiting','Online') collate latin1_general_ci NOT NULL default 'Offline',
  `Privilege` varchar(255) collate latin1_general_ci NOT NULL default '',
  `ExtendedPrivilege` varchar(45) collate latin1_general_ci NOT NULL default '',
  `Cookie` varchar(255) collate latin1_general_ci NOT NULL default '',
  PRIMARY KEY  (`UId`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci COMMENT='contains all users information for login system';

