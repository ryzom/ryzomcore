# MySQL-Front Dump 2.4
#
# Host: localhost   Database: ring_ats
#--------------------------------------------------------
# Server version 4.0.24_Debian-10sarge1-log

USE ring_ats;


#
# Table structure for table 'characters'
#

CREATE TABLE `characters` (
  `char_id` int(10) unsigned NOT NULL default '0',
  `char_name` varchar(20) NOT NULL default '',
  `user_id` int(10) unsigned NOT NULL default '0',
  `guild_id` int(10) unsigned NOT NULL default '0',
  `best_combat_level` int(10) unsigned NOT NULL default '0',
  `home_mainland_session_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`char_id`),
  UNIQUE KEY `char_name_idx` (`char_name`),
  KEY `user_id_idx` (`user_id`),
  KEY `guild_id_idx` (`guild_id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'folder'
#

CREATE TABLE `folder` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `owner` int(10) unsigned NOT NULL default '0',
  `title` varchar(40) NOT NULL default '',
  `comments` text NOT NULL,
  PRIMARY KEY  (`Id`),
  KEY `owner_idx` (`owner`),
  KEY `title_idx` (`title`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'folder_access'
#

CREATE TABLE `folder_access` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `folder_id` int(10) unsigned NOT NULL default '0',
  `user_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`),
  KEY `folder_id_idx` (`folder_id`),
  KEY `user_idx` (`user_id`)
) TYPE=MyISAM ROW_FORMAT=FIXED;



#
# Table structure for table 'guild_invites'
#

CREATE TABLE `guild_invites` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `session_id` int(10) unsigned NOT NULL default '0',
  `guild_id` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`),
  KEY `guild_id_idx` (`guild_id`),
  KEY `session_id_idx` (`session_id`)
) TYPE=MyISAM ROW_FORMAT=FIXED;



#
# Table structure for table 'guilds'
#

CREATE TABLE `guilds` (
  `guild_id` int(10) unsigned NOT NULL default '0',
  `guild_name` varchar(20) NOT NULL default '',
  `shard_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`guild_id`),
  UNIQUE KEY `huild_name_idx` (`guild_name`),
  KEY `shard_id_idx` (`shard_id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'journal_entry'
#

CREATE TABLE `journal_entry` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `session_id` int(10) unsigned NOT NULL default '0',
  `author` int(10) unsigned NOT NULL default '0',
  `type` enum('jet_credits','jet_notes') NOT NULL default 'jet_notes',
  `text` text NOT NULL,
  `time_stamp` datetime NOT NULL default '2005-09-07 12:41:33',
  PRIMARY KEY  (`Id`),
  KEY `session_id_idx` (`session_id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'known_users'
#

CREATE TABLE `known_users` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `owner` int(10) unsigned NOT NULL default '0',
  `targer_user` int(10) unsigned NOT NULL default '0',
  `targer_character` int(10) unsigned NOT NULL default '0',
  `relation_type` enum('rt_friend','rt_banned','rt_friend_dm') NOT NULL default 'rt_friend',
  `comments` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`Id`),
  KEY `user_index` (`owner`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'mfs_erased_mail_series'
#

CREATE TABLE `mfs_erased_mail_series` (
  `erased_char_id` int(11) unsigned NOT NULL default '0',
  `erased_char_name` varchar(32) NOT NULL default '',
  `erased_series` int(11) unsigned NOT NULL auto_increment,
  `erase_date` datetime NOT NULL default '0000-00-00 00:00:00',
  PRIMARY KEY  (`erased_series`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'mfs_guild_thread'
#

CREATE TABLE `mfs_guild_thread` (
  `thread_id` int(11) NOT NULL auto_increment,
  `guild_id` int(11) unsigned NOT NULL default '0',
  `topic` varchar(255) NOT NULL default '',
  `author_name` varchar(32) NOT NULL default '',
  `last_post_date` datetime NOT NULL default '0000-00-00 00:00:00',
  `post_count` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`thread_id`),
  KEY `guild_index` (`guild_id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'mfs_guild_thread_message'
#

CREATE TABLE `mfs_guild_thread_message` (
  `id` int(11) NOT NULL auto_increment,
  `thread_id` int(11) unsigned NOT NULL default '0',
  `author_name` varchar(32) NOT NULL default '',
  `date` datetime NOT NULL default '0000-00-00 00:00:00',
  `content` text NOT NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'mfs_mail'
#

CREATE TABLE `mfs_mail` (
  `id` int(11) NOT NULL auto_increment,
  `sender_name` varchar(32) NOT NULL default '',
  `subject` varchar(250) NOT NULL default '',
  `date` datetime NOT NULL default '0000-00-00 00:00:00',
  `status` enum('ms_new','ms_read','ms_erased') NOT NULL default 'ms_new',
  `dest_char_id` int(11) unsigned NOT NULL default '0',
  `erase_series` int(11) unsigned NOT NULL default '0',
  `content` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `dest_index` (`dest_char_id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'player_rating'
#

CREATE TABLE `player_rating` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `session_id` int(10) unsigned NOT NULL default '0',
  `author` int(10) unsigned NOT NULL default '0',
  `rating` int(10) NOT NULL default '0',
  `comments` text NOT NULL,
  `time_stamp` datetime NOT NULL default '2005-09-07 12:41:33',
  PRIMARY KEY  (`Id`),
  KEY `session_id_idx` (`session_id`),
  KEY `author_idx` (`author`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'ring_users'
#

CREATE TABLE `ring_users` (
  `user_id` int(10) unsigned NOT NULL default '0',
  `user_name` varchar(20) NOT NULL default '',
  `user_type` enum('ut_character','ut_pioneer') NOT NULL default 'ut_character',
  `current_session` int(10) unsigned NOT NULL default '0',
  `current_activity` enum('ca_none','ca_play','ca_edit','ca_anim') NOT NULL default 'ca_none',
  `current_status` enum('cs_offline','cs_logged','cs_online') NOT NULL default 'cs_offline',
  `public_level` enum('pl_none','pl_public') NOT NULL default 'pl_none',
  `account_type` enum('at_normal','at_gold') NOT NULL default 'at_normal',
  `content_access_level` varchar(20) NOT NULL default '',
  `description` text NOT NULL,
  `lang` enum('lang_en','lang_fr','lang_de') NOT NULL default 'lang_en',
  `cookie` varchar(30) NOT NULL default '',
  `current_domain_id` int(10) NOT NULL default '-1',
  `pioneer_char_id` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`user_id`),
  UNIQUE KEY `user_name_idx` (`user_name`),
  KEY `cookie_idx` (`cookie`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'scenario_desc'
#

CREATE TABLE `scenario_desc` (
  `session_id` int(10) unsigned NOT NULL default '0',
  `parent_scenario` int(10) unsigned NOT NULL default '0',
  `description` text NOT NULL,
  `relation_to_parent` enum('rtp_same','rtp_variant','rtp_different') NOT NULL default 'rtp_same',
  `title` varchar(40) NOT NULL default '',
  `num_player` int(10) unsigned NOT NULL default '0',
  `content_access_level` varchar(20) NOT NULL default '',
  PRIMARY KEY  (`session_id`),
  UNIQUE KEY `title_idx` (`title`),
  KEY `parent_idx` (`parent_scenario`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'session_participant'
#

CREATE TABLE `session_participant` (
  `Id` int(10) unsigned NOT NULL auto_increment,
  `session_id` int(10) unsigned NOT NULL default '0',
  `char_id` int(10) unsigned NOT NULL default '0',
  `status` enum('sps_play_subscribed','sps_play_invited','sps_edit_invited','sps_anim_invited','sps_playing','sps_editing','sps_animating') NOT NULL default 'sps_play_subscribed',
  `kicked` tinyint(1) unsigned NOT NULL default '0',
  `session_rated` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`Id`),
  KEY `session_idx` (`session_id`),
  KEY `user_idx` (`char_id`)
) TYPE=MyISAM ROW_FORMAT=FIXED;



#
# Table structure for table 'sessions'
#

CREATE TABLE `sessions` (
  `session_id` int(10) unsigned NOT NULL auto_increment,
  `session_type` enum('st_edit','st_anim','st_outland','st_mainland') NOT NULL default 'st_edit',
  `title` varchar(40) NOT NULL default '',
  `owner` int(10) unsigned NOT NULL default '0',
  `plan_date` datetime NOT NULL default '2005-09-21 12:41:33',
  `start_date` datetime NOT NULL default '2005-08-31 00:00:00',
  `description` text NOT NULL,
  `orientation` enum('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') NOT NULL default 'so_other',
  `level` enum('sl_a','sl_b','sl_c','sl_d','sl_e') NOT NULL default 'sl_a',
  `rule_type` enum('rt_strict','rt_liberal') NOT NULL default 'rt_strict',
  `access_type` enum('at_public','at_private') NOT NULL default 'at_public',
  `state` enum('ss_planned','ss_open','ss_locked','ss_closed') NOT NULL default 'ss_planned',
  `host_shard_id` int(11) NOT NULL default '0',
  `subscription_slots` int(11) unsigned NOT NULL default '0',
  `reserved_slots` int(10) unsigned NOT NULL default '0',
  `free_slots` int(10) unsigned NOT NULL default '0',
  `estimated_duration` enum('et_short','et_medium','et_long') NOT NULL default 'et_short',
  `final_duration` int(10) unsigned NOT NULL default '0',
  `folder_id` int(10) unsigned NOT NULL default '0',
  `lang` enum('lang_en','lang_fr','lang_de') NOT NULL default 'lang_en',
  `icone` varchar(70) NOT NULL default '',
  PRIMARY KEY  (`session_id`),
  KEY `owner_idx` (`owner`),
  KEY `folder_idx` (`folder_id`)
) TYPE=MyISAM ROW_FORMAT=DYNAMIC;



#
# Table structure for table 'shard'
#

CREATE TABLE `shard` (
  `shard_id` int(11) NOT NULL default '0',
  PRIMARY KEY  (`shard_id`)
) TYPE=MyISAM ROW_FORMAT=FIXED;

