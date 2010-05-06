# MySQL-Front Dump 2.4
#
# Host: localhost   Database: nel_tool
#--------------------------------------------------------
# Server version 4.0.24_Debian-10sarge1-log

USE nel_tool;


#
# Table structure for table 'help_topic'
#

CREATE TABLE `help_topic` (
  `file` varchar(32) default '0',
  `topic` varchar(32) default '0',
  `help_body` text
) TYPE=MyISAM;



#
# Table structure for table 'server'
#

CREATE TABLE `server` (
  `name` char(32) NOT NULL default '0',
  `address` char(32) NOT NULL default '0'
) TYPE=MyISAM;



#
# Table structure for table 'service'
#

CREATE TABLE `service` (
  `service_id` int(10) unsigned NOT NULL auto_increment,
  `shard` char(32) NOT NULL default '',
  `server` char(32) NOT NULL default '',
  `name` char(32) NOT NULL default '',
  PRIMARY KEY  (`service_id`),
  UNIQUE KEY `service_id` (`service_id`),
  KEY `service_id_2` (`service_id`)
) TYPE=MyISAM;



#
# Table structure for table 'shard_access'
#

CREATE TABLE `shard_access` (
  `uid` int(10) unsigned default '0',
  `shard` char(64) default '0'
) TYPE=MyISAM;



#
# Table structure for table 'shard_annotation'
#

CREATE TABLE `shard_annotation` (
  `shard` varchar(32) default '0',
  `annotation` varchar(255) default '0',
  `user` int(10) unsigned default '0',
  `post_date` datetime default NULL,
  `lock_user` int(10) unsigned default '0',
  `lock_ip` varchar(32) default NULL,
  `lock_date` datetime default NULL,
  `ASAddr` varchar(255) default NULL,
  `alias` varchar(255) default NULL
) TYPE=MyISAM;



#
# Table structure for table 'user'
#

CREATE TABLE `user` (
  `login` varchar(16) NOT NULL default '',
  `password` varchar(32) NOT NULL default '',
  `uid` int(10) NOT NULL auto_increment,
  `gid` int(10) NOT NULL default '1',
  `useCookie` enum('yes','no') NOT NULL default 'no',
  `default_view` int(11) NOT NULL default '0',
  `allowed_ip` varchar(32) default NULL,
  PRIMARY KEY  (`uid`),
  UNIQUE KEY `login` (`login`)
) TYPE=MyISAM;



#
# Table structure for table 'user_right'
#

CREATE TABLE `user_right` (
  `uid` int(10) unsigned default '0',
  `uright` varchar(16) default '0',
  KEY `uid` (`uid`)
) TYPE=MyISAM;



#
# Table structure for table 'user_variable'
#

CREATE TABLE `user_variable` (
  `uid` int(11) NOT NULL default '0',
  `vid` int(11) NOT NULL default '0',
  `privilege` enum('none','rd','rw') NOT NULL default 'none',
  PRIMARY KEY  (`uid`,`vid`),
  UNIQUE KEY `uid` (`uid`,`vid`)
) TYPE=MyISAM;



#
# Table structure for table 'variable'
#

CREATE TABLE `variable` (
  `vid` int(11) NOT NULL auto_increment,
  `name` varchar(128) NOT NULL default '',
  `path` varchar(255) NOT NULL default '',
  `state` enum('rd','rw') NOT NULL default 'rd',
  `vgid` int(10) unsigned NOT NULL default '1',
  `warning_bound` int(11) default '-1',
  `error_bound` int(11) default '-1',
  `alarm_order` enum('gt','lt') NOT NULL default 'gt',
  `graph_update` int(10) unsigned default '0',
  `command` enum('variable','command') NOT NULL default 'variable',
  PRIMARY KEY  (`vid`),
  UNIQUE KEY `vid` (`vid`)
) TYPE=MyISAM;



#
# Table structure for table 'variable_group'
#

CREATE TABLE `variable_group` (
  `vgid` int(10) NOT NULL auto_increment,
  `name` varchar(32) default '0',
  PRIMARY KEY  (`vgid`),
  UNIQUE KEY `name` (`name`)
) TYPE=MyISAM;



#
# Table structure for table 'view_command'
#

CREATE TABLE `view_command` (
  `name` varchar(32) default '0',
  `command` varchar(32) default '0',
  `tid` int(11) unsigned default '0'
) TYPE=MyISAM;



#
# Table structure for table 'view_row'
#

CREATE TABLE `view_row` (
  `tid` int(11) NOT NULL default '0',
  `vid` int(11) NOT NULL default '0',
  `name` varchar(128) NOT NULL default '',
  `ordering` tinyint(4) NOT NULL default '0',
  `filter` varchar(64) default NULL,
  `graph` tinyint(3) unsigned NOT NULL default '0'
) TYPE=MyISAM;



#
# Table structure for table 'view_table'
#

CREATE TABLE `view_table` (
  `tid` int(11) NOT NULL auto_increment,
  `uid` int(11) NOT NULL default '0',
  `name` varchar(32) NOT NULL default '',
  `ordering` tinyint(4) NOT NULL default '0',
  `filter` varchar(64) default NULL,
  `display` enum('normal','condensed') NOT NULL default 'normal',
  `refresh_rate` int(10) unsigned default '0',
  `auto_display` enum('auto','manual') NOT NULL default 'auto',
  `show_base_cols` tinyint(3) unsigned NOT NULL default '1',
  UNIQUE KEY `tid` (`tid`,`uid`)
) TYPE=MyISAM;

