-- phpMyAdmin SQL Dump
-- version 3.4.10.1deb1
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 14, 2014 at 10:03 AM
-- Server version: 5.5.37
-- PHP Version: 5.3.10-1ubuntu3.11

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `nel_tool`
--

-- --------------------------------------------------------

--
-- Table structure for table `neltool_annotations`
--

CREATE TABLE IF NOT EXISTS `neltool_annotations` (
  `annotation_id` int(11) NOT NULL AUTO_INCREMENT,
  `annotation_domain_id` int(11) DEFAULT NULL,
  `annotation_shard_id` int(11) DEFAULT NULL,
  `annotation_data` varchar(255) NOT NULL DEFAULT '',
  `annotation_user_name` varchar(32) NOT NULL DEFAULT '',
  `annotation_date` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`annotation_id`),
  UNIQUE KEY `annotation_shard_id` (`annotation_shard_id`),
  UNIQUE KEY `annotation_domain_id` (`annotation_domain_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=2 ;

--
-- Dumping data for table `neltool_annotations`
--

INSERT INTO `neltool_annotations` (`annotation_id`, `annotation_domain_id`, `annotation_shard_id`, `annotation_data`, `annotation_user_name`, `annotation_date`) VALUES
(1, NULL, 106, 'Welcome to the Shard Admin Website!', 'vl', 1272378352);

-- --------------------------------------------------------

--
-- Table structure for table `neltool_applications`
--

CREATE TABLE IF NOT EXISTS `neltool_applications` (
  `application_id` int(11) NOT NULL AUTO_INCREMENT,
  `application_name` varchar(64) NOT NULL DEFAULT '',
  `application_uri` varchar(255) NOT NULL DEFAULT '',
  `application_restriction` varchar(64) NOT NULL DEFAULT '',
  `application_order` int(11) NOT NULL DEFAULT '0',
  `application_visible` int(11) NOT NULL DEFAULT '0',
  `application_icon` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`application_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=40 ;

--
-- Dumping data for table `neltool_applications`
--

INSERT INTO `neltool_applications` (`application_id`, `application_name`, `application_uri`, `application_restriction`, `application_order`, `application_visible`, `application_icon`) VALUES
(1, 'Main', 'index.php', '', 100, 1, 'imgs/icon_main.gif'),
(2, 'Logout', 'index.php?mode=logout', '', 999999, 1, 'imgs/icon_logout.gif'),
(3, 'Admin', 'tool_administration.php', 'tool_admin', 1500, 1, 'imgs/icon_admin.gif'),
(4, 'Prefs', 'tool_preferences.php', 'tool_preferences', 1000, 1, 'imgs/icon_preferences.gif'),
(5, 'Admin/Users', '', 'tool_admin_user', 1502, 0, ''),
(6, 'Admin/Applications', '', 'tool_admin_application', 1501, 0, ''),
(7, 'Admin/Domains', '', 'tool_admin_domain', 1504, 0, ''),
(8, 'Admin/Shards', '', 'tool_admin_shard', 1505, 0, ''),
(9, 'Admin/Groups', '', 'tool_admin_group', 1503, 0, ''),
(10, 'Admin/Logs', '', 'tool_admin_logs', 1506, 0, ''),
(11, 'Main/Start', '', 'tool_main_start', 101, 0, ''),
(12, 'Main/Stop', '', 'tool_main_stop', 102, 0, ''),
(13, 'Main/Restart', '', 'tool_main_restart', 103, 0, ''),
(14, 'Main/Kill', '', 'tool_main_kill', 104, 0, ''),
(15, 'Main/Abort', '', 'tool_main_abort', 105, 0, ''),
(16, 'Main/Execute', '', 'tool_main_execute', 108, 0, ''),
(18, 'Notes', 'tool_notes.php', 'tool_notes', 900, 1, 'imgs/icon_notes.gif'),
(19, 'Player Locator', 'tool_player_locator.php', 'tool_player_locator', 200, 1, 'imgs/icon_player_locator.gif'),
(20, 'Player Locator/Display Players', '', 'tool_player_locator_display_players', 201, 0, ''),
(21, 'Player Locator/Locate', '', 'tool_player_locator_locate', 202, 0, ''),
(22, 'Main/LockDomain', '', 'tool_main_lock_domain', 110, 0, ''),
(23, 'Main/LockShard', '', 'tool_main_lock_shard', 111, 0, ''),
(24, 'Main/WS', '', 'tool_main_ws', 112, 0, ''),
(25, 'Main/ResetCounters', '', 'tool_main_reset_counters', 113, 0, ''),
(26, 'Main/ServiceAutoStart', '', 'tool_main_service_autostart', 114, 0, ''),
(27, 'Main/ShardAutoStart', '', 'tool_main_shard_autostart', 115, 0, ''),
(28, 'Main/WS/Old', '', 'tool_main_ws_old', 112, 0, ''),
(29, 'Graphs', 'tool_graphs.php', 'tool_graph', 500, 1, 'imgs/icon_graphs.gif'),
(30, 'Notes/Global', '', 'tool_notes_global', 901, 0, ''),
(31, 'Log Analyser', 'tool_log_analyser.php', 'tool_las', 400, 1, 'imgs/icon_log_analyser.gif'),
(32, 'Guild Locator', 'tool_guild_locator.php', 'tool_guild_locator', 300, 1, 'imgs/icon_guild_locator.gif'),
(33, 'Player Locator/UserID Check', '', 'tool_player_locator_userid_check', 203, 0, ''),
(34, 'Player Locator/CSR Relocate', '', 'tool_player_locator_csr_relocate', 204, 0, ''),
(35, 'Guild Locator/Guilds Update', '', 'tool_guild_locator_manage_guild', 301, 0, ''),
(36, 'Guild Locator/Members Update', '', 'tool_guild_locator_manage_members', 302, 0, ''),
(37, 'Entities', 'tool_event_entities.php', 'tool_event_entities', 350, 1, 'imgs/icon_entity.gif'),
(38, 'Admin/Restarts', '', 'tool_admin_restart', 1507, 0, ''),
(39, 'Main/EasyRestart', '', 'tool_main_easy_restart', 116, 0, '');

-- --------------------------------------------------------

--
-- Table structure for table `neltool_domains`
--

CREATE TABLE IF NOT EXISTS `neltool_domains` (
  `domain_id` int(11) NOT NULL AUTO_INCREMENT,
  `domain_name` varchar(128) NOT NULL DEFAULT '',
  `domain_as_host` varchar(128) NOT NULL DEFAULT '',
  `domain_as_port` int(11) NOT NULL DEFAULT '0',
  `domain_rrd_path` varchar(255) NOT NULL DEFAULT '',
  `domain_las_admin_path` varchar(255) NOT NULL DEFAULT '',
  `domain_las_local_path` varchar(255) NOT NULL DEFAULT '',
  `domain_application` varchar(128) NOT NULL DEFAULT '',
  `domain_sql_string` varchar(128) NOT NULL DEFAULT '',
  `domain_hd_check` int(11) NOT NULL DEFAULT '0',
  `domain_mfs_web` text,
  `domain_cs_sql_string` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`domain_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_groups`
--

CREATE TABLE IF NOT EXISTS `neltool_groups` (
  `group_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_name` varchar(32) NOT NULL DEFAULT 'NewGroup',
  `group_level` int(11) NOT NULL DEFAULT '0',
  `group_default` int(11) NOT NULL DEFAULT '0',
  `group_active` int(11) NOT NULL DEFAULT '0',
  `group_default_domain_id` tinyint(3) unsigned DEFAULT NULL,
  `group_default_shard_id` smallint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`group_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=10 ;

--
-- Dumping data for table `neltool_groups`
--

INSERT INTO `neltool_groups` (`group_id`, `group_name`, `group_level`, `group_default`, `group_active`, `group_default_domain_id`, `group_default_shard_id`) VALUES
(1, 'AdminGroup', 0, 0, 1, 20, 300),
(2, 'DeveloperGroup', 0, 1, 1, 20, 300),
(3, 'AdminDebugGroup', 10, 0, 1, 20, 300),
(4, 'SupportSGMGroup', 0, 0, 1, NULL, NULL),
(6, 'SupportGMGroup', 0, 0, 1, NULL, NULL),
(7, 'SupportReadOnlyGroup', 0, 0, 1, NULL, NULL),
(8, 'DeveloperLevelDesigners', 0, 0, 1, 20, 300),
(9, 'DeveloperReadOnlyGroup', 0, 0, 1, 20, 300);

-- --------------------------------------------------------

--
-- Table structure for table `neltool_group_applications`
--

CREATE TABLE IF NOT EXISTS `neltool_group_applications` (
  `group_application_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_application_group_id` int(11) NOT NULL DEFAULT '0',
  `group_application_application_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`group_application_id`),
  KEY `group_application_group_id` (`group_application_group_id`),
  KEY `group_application_application_id` (`group_application_application_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=966 ;

--
-- Dumping data for table `neltool_group_applications`
--

INSERT INTO `neltool_group_applications` (`group_application_id`, `group_application_group_id`, `group_application_application_id`) VALUES
(879, 1, 10),
(878, 1, 8),
(877, 1, 7),
(876, 1, 9),
(875, 1, 5),
(874, 1, 6),
(873, 1, 3),
(872, 1, 4),
(871, 1, 30),
(870, 1, 18),
(869, 1, 29),
(868, 1, 31),
(867, 1, 37),
(866, 1, 36),
(865, 1, 35),
(864, 1, 32),
(863, 1, 34),
(862, 1, 33),
(861, 1, 21),
(860, 1, 20),
(859, 1, 19),
(858, 1, 39),
(857, 1, 27),
(856, 1, 26),
(843, 3, 10),
(842, 3, 8),
(841, 3, 7),
(840, 3, 9),
(839, 3, 5),
(838, 3, 6),
(837, 3, 3),
(836, 3, 4),
(835, 3, 30),
(834, 3, 18),
(833, 3, 29),
(832, 3, 31),
(831, 3, 37),
(830, 3, 36),
(829, 3, 35),
(828, 3, 32),
(827, 3, 34),
(826, 3, 33),
(825, 3, 21),
(824, 3, 20),
(823, 3, 19),
(822, 3, 39),
(821, 3, 27),
(820, 3, 26),
(597, 4, 36),
(596, 4, 35),
(595, 4, 32),
(594, 4, 21),
(593, 4, 20),
(592, 4, 19),
(591, 4, 24),
(590, 4, 23),
(589, 4, 14),
(588, 4, 12),
(632, 2, 18),
(631, 2, 37),
(630, 2, 32),
(629, 2, 21),
(628, 2, 20),
(627, 2, 19),
(626, 2, 24),
(625, 2, 23),
(624, 2, 22),
(623, 2, 16),
(622, 2, 15),
(621, 2, 14),
(620, 2, 13),
(819, 3, 25),
(855, 1, 25),
(619, 2, 12),
(818, 3, 28),
(854, 1, 28),
(817, 3, 24),
(718, 5, 18),
(717, 5, 37),
(716, 5, 32),
(715, 5, 21),
(714, 5, 20),
(713, 5, 19),
(712, 5, 27),
(711, 5, 26),
(710, 5, 24),
(709, 5, 23),
(708, 5, 22),
(707, 5, 16),
(706, 5, 15),
(705, 5, 14),
(816, 3, 23),
(609, 6, 35),
(608, 6, 32),
(607, 6, 21),
(606, 6, 20),
(605, 6, 19),
(604, 6, 24),
(603, 6, 23),
(602, 6, 14),
(601, 6, 12),
(600, 6, 11),
(815, 3, 22),
(814, 3, 16),
(853, 1, 24),
(704, 5, 13),
(703, 5, 12),
(852, 1, 23),
(587, 4, 11),
(618, 2, 11),
(702, 5, 11),
(612, 7, 19),
(851, 1, 22),
(813, 3, 15),
(812, 3, 14),
(598, 4, 18),
(599, 4, 4),
(610, 6, 18),
(611, 6, 4),
(613, 7, 20),
(614, 7, 21),
(615, 7, 32),
(616, 7, 35),
(617, 7, 4),
(633, 2, 4),
(811, 3, 13),
(810, 3, 12),
(850, 1, 16),
(849, 1, 15),
(848, 1, 14),
(847, 1, 13),
(846, 1, 12),
(719, 5, 4),
(720, 8, 11),
(721, 8, 12),
(722, 8, 13),
(723, 8, 14),
(724, 8, 15),
(725, 8, 16),
(726, 8, 22),
(727, 8, 23),
(728, 8, 24),
(729, 8, 25),
(730, 8, 26),
(731, 8, 27),
(732, 8, 19),
(733, 8, 20),
(734, 8, 21),
(735, 8, 37),
(736, 8, 4),
(737, 9, 29),
(738, 9, 4),
(809, 3, 11),
(845, 1, 11),
(844, 3, 38),
(880, 1, 38),
(909, 10, 18),
(908, 10, 29),
(907, 10, 37),
(906, 10, 36),
(905, 10, 35),
(904, 10, 32),
(903, 10, 34),
(902, 10, 33),
(901, 10, 21),
(900, 10, 20),
(899, 10, 19),
(898, 10, 23),
(897, 10, 13),
(910, 10, 30),
(965, 11, 29),
(964, 11, 37),
(963, 11, 32),
(962, 11, 34),
(961, 11, 33),
(960, 11, 21),
(959, 11, 20),
(958, 11, 19);

-- --------------------------------------------------------

--
-- Table structure for table `neltool_group_domains`
--

CREATE TABLE IF NOT EXISTS `neltool_group_domains` (
  `group_domain_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_domain_group_id` int(11) NOT NULL DEFAULT '0',
  `group_domain_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`group_domain_id`),
  KEY `group_domain_group_id` (`group_domain_group_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_group_shards`
--

CREATE TABLE IF NOT EXISTS `neltool_group_shards` (
  `group_shard_id` int(11) NOT NULL AUTO_INCREMENT,
  `group_shard_group_id` int(11) NOT NULL DEFAULT '0',
  `group_shard_shard_id` int(11) NOT NULL DEFAULT '0',
  `group_shard_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`group_shard_id`),
  KEY `group_shard_group_id` (`group_shard_group_id`),
  KEY `group_shard_domain_id` (`group_shard_domain_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

--
-- Dumping data for table `neltool_group_shards`
--

-- --------------------------------------------------------

--
-- Table structure for table `neltool_locks`
--

CREATE TABLE IF NOT EXISTS `neltool_locks` (
  `lock_id` int(11) NOT NULL AUTO_INCREMENT,
  `lock_domain_id` int(11) DEFAULT NULL,
  `lock_shard_id` int(11) DEFAULT NULL,
  `lock_user_name` varchar(32) NOT NULL DEFAULT '',
  `lock_date` int(11) NOT NULL DEFAULT '0',
  `lock_update` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`lock_id`),
  UNIQUE KEY `lock_shard_id` (`lock_shard_id`),
  UNIQUE KEY `lock_domain_id` (`lock_domain_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_logs`
--

CREATE TABLE IF NOT EXISTS `neltool_logs` (
  `logs_id` int(11) NOT NULL AUTO_INCREMENT,
  `logs_user_name` varchar(32) NOT NULL DEFAULT '0',
  `logs_date` int(11) NOT NULL DEFAULT '0',
  `logs_data` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`logs_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_notes`
--

CREATE TABLE IF NOT EXISTS `neltool_notes` (
  `note_id` int(11) NOT NULL AUTO_INCREMENT,
  `note_user_id` int(11) NOT NULL DEFAULT '0',
  `note_title` varchar(128) NOT NULL DEFAULT '',
  `note_data` text NOT NULL,
  `note_date` int(11) NOT NULL DEFAULT '0',
  `note_active` int(11) NOT NULL DEFAULT '0',
  `note_global` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`note_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=11 ;

--
-- Dumping data for table `neltool_notes`
--

INSERT INTO `neltool_notes` (`note_id`, `note_user_id`, `note_title`, `note_data`, `note_date`, `note_active`, `note_global`) VALUES
(2, 27, 'Welcome', 'Welcome to the shard administration website!\r\n\r\nThis website is used to monitor and restart shards.\r\n\r\nIt also gives some player characters information.', 1272378065, 1, 1),
(3, 27, 'Shard Start', '# At the same time : NS and TS\r\n[1 min] : all MS, you can boot them all at the same time\r\n[1 min] : IOS\r\n[3 mins] : GMPS\r\n[3 mins] : EGS\r\n[5 mins] : AI Fyros\r\n[1 min 30] : AI Zorai\r\n[1 min 30] : AI Matis\r\n[1 min 30] : AI TNP\r\n[1 min 30] : AI NPE\r\n[1 min 30] : AI Tryker\r\n[1 min 30] : All FS and SBS at the same time\r\n[30 secs] : WS (atm the WS starts in OPEN mode by default, so be fast before CSR checkage, fix for that inc soon)\r\n\r\nNOTE: you can check the uptime for those timers in the right column of the admin tool: UpTime\r\n', 1158751126, 1, 0),
(5, 27, 'shutting supplementary', 'the writing wont change when lock the ws\r\n\r\nuntick previous boxes as you shut down\r\n\r\nwait 5 between the ws and the egs ie egs is 5 past rest is 10 past', 1153395380, 1, 0),
(4, 27, 'Shard Stop', '1. Broadcast to warn players\r\n\r\n2. 10 mins before shutdown, lock the WS\r\n\r\n3. At the right time shut down WS\r\n\r\n4. Shut down EGS\r\nOnly the EGS. Wait 5 reals minutes. Goal is to give enough time to egs, in order to save all the info he has to, and letting him sending those message to all services who need it.\r\n\r\n5. Shut down the rest, et voil&agrave;, you&#039;re done.', 1153314198, 1, 0),
(6, 27, 'Start (EGS to high?)', 'If [EGS] is to high on startup:\r\n\r\n[shut down egs]\r\n[5 mins]\r\n\r\n[IOS] &amp; [GPMS] (shut down at same time)\r\n\r\nAfter the services are down follow &quot;UP&quot; process with timers again.\r\n\r\nIOS\r\n[3 mins]\r\nGPMS\r\n[3 mins]\r\nEGS\r\n[5 mins]\r\nbla bla...', 1153395097, 1, 0),
(7, 27, 'opening if the egs is too high on reboot', '&lt;kadael&gt; here my note on admin about egs to high on startup\r\n&lt;kadael&gt; ---\r\n&lt;kadael&gt; If [EGS] is to high on startup:\r\n&lt;kadael&gt; [shut down egs]\r\n&lt;kadael&gt; [5 mins]\r\n&lt;kadael&gt; [IOS] &amp; [GPMS] (at same time shut down )\r\n&lt;kadael&gt; after the services are down follow &quot;UP&quot; process with timers again.\r\n&lt;kadael&gt; IOS\r\n&lt;kadael&gt; [3 mins]\r\n&lt;kadael&gt; GPMS\r\n&lt;kadael&gt; [3 mins]\r\n&lt;kadael&gt; EGS\r\n&lt;kadael&gt; [5 mins]\r\n&lt;kadael&gt; bla bla...\r\n&lt;kadael&gt; ---', 1153395362, 1, 0),
(10, 27, 'Ring points', 'Commande pour donner tout les points ring &agrave; tout le monde :\r\n\r\nDans le DSS d&#039;un Shard Ring entrer : DefaultCharRingAccess f7:j7:l6:d7:p13:g9:a9', 1155722296, 1, 0),
(9, 27, 'Start (EGS to high?)', 'If [EGS] is to high on startup: \r\n  \r\n [shut down egs] \r\n [5 mins] \r\n  \r\n [IOS] &amp; [GPMS] (shut down at same time) \r\n  \r\n After the services are down follow &quot;UP&quot; process with timers again. \r\n  \r\n IOS \r\n [3 mins] \r\n GPMS \r\n [3 mins] \r\n EGS \r\n [5 mins] \r\n bla bla...', 1153929658, 1, 0);

-- --------------------------------------------------------

--
-- Table structure for table `neltool_restart_groups`
--

CREATE TABLE IF NOT EXISTS `neltool_restart_groups` (
  `restart_group_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `restart_group_name` varchar(50) DEFAULT NULL,
  `restart_group_list` varchar(50) DEFAULT NULL,
  `restart_group_order` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`restart_group_id`),
  UNIQUE KEY `restart_group_id` (`restart_group_id`),
  KEY `restart_group_id_2` (`restart_group_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=6 ;

--
-- Dumping data for table `neltool_restart_groups`
--

INSERT INTO `neltool_restart_groups` (`restart_group_id`, `restart_group_name`, `restart_group_list`, `restart_group_order`) VALUES
(1, 'Low Level', 'rns,ts,ms', '1'),
(3, 'Mid Level', 'ios,gpms,egs', '2'),
(4, 'High Level', 'ais', '3'),
(5, 'Front Level', 'fes,sbs,dss,rws', '4');

-- --------------------------------------------------------

--
-- Table structure for table `neltool_restart_messages`
--

CREATE TABLE IF NOT EXISTS `neltool_restart_messages` (
  `restart_message_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `restart_message_name` varchar(20) DEFAULT NULL,
  `restart_message_value` varchar(128) DEFAULT NULL,
  `restart_message_lang` varchar(5) DEFAULT NULL,
  PRIMARY KEY (`restart_message_id`),
  UNIQUE KEY `restart_message_id` (`restart_message_id`),
  KEY `restart_message_id_2` (`restart_message_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=11 ;

--
-- Dumping data for table `neltool_restart_messages`
--

INSERT INTO `neltool_restart_messages` (`restart_message_id`, `restart_message_name`, `restart_message_value`, `restart_message_lang`) VALUES
(5, 'reboot', 'The shard is about to go down. Please find a safe location and log out.', 'en'),
(4, 'reboot', 'Le serveur va redemarrer dans $minutes$ minutes. Merci de vous deconnecter en lieu sur.', 'fr'),
(6, 'reboot', 'Der Server wird heruntergefahren. Findet eine sichere Stelle und logt aus.', 'de'),
(10, 'reboot', 'Arret du serveur dans $minutes+1$ minutes', 'fr');

-- --------------------------------------------------------

--
-- Table structure for table `neltool_restart_sequences`
--

CREATE TABLE IF NOT EXISTS `neltool_restart_sequences` (
  `restart_sequence_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `restart_sequence_domain_id` int(10) unsigned NOT NULL DEFAULT '0',
  `restart_sequence_shard_id` int(10) unsigned NOT NULL DEFAULT '0',
  `restart_sequence_user_name` varchar(50) DEFAULT NULL,
  `restart_sequence_step` int(10) unsigned NOT NULL DEFAULT '0',
  `restart_sequence_date_start` int(11) DEFAULT NULL,
  `restart_sequence_date_end` int(11) DEFAULT NULL,
  `restart_sequence_timer` int(11) unsigned DEFAULT '0',
  PRIMARY KEY (`restart_sequence_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_shards`
--

CREATE TABLE IF NOT EXISTS `neltool_shards` (
  `shard_id` int(11) NOT NULL AUTO_INCREMENT,
  `shard_name` varchar(128) NOT NULL DEFAULT '',
  `shard_as_id` varchar(255) NOT NULL DEFAULT '0',
  `shard_domain_id` int(11) NOT NULL DEFAULT '0',
  `shard_lang` char(2) NOT NULL DEFAULT 'en',
  `shard_restart` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`shard_id`),
  KEY `shard_domain_id` (`shard_domain_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_stats_hd_datas`
--

CREATE TABLE IF NOT EXISTS `neltool_stats_hd_datas` (
  `hd_id` int(11) NOT NULL AUTO_INCREMENT,
  `hd_domain_id` int(11) NOT NULL DEFAULT '0',
  `hd_server` varchar(32) NOT NULL DEFAULT '',
  `hd_device` varchar(64) NOT NULL DEFAULT '',
  `hd_size` varchar(16) NOT NULL DEFAULT '',
  `hd_used` varchar(16) NOT NULL DEFAULT '',
  `hd_free` varchar(16) NOT NULL DEFAULT '',
  `hd_percent` int(11) NOT NULL DEFAULT '0',
  `hd_mount` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`hd_id`),
  KEY `hd_domain_id` (`hd_domain_id`),
  KEY `hd_server` (`hd_server`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_stats_hd_times`
--

CREATE TABLE IF NOT EXISTS `neltool_stats_hd_times` (
  `hd_domain_id` int(11) NOT NULL DEFAULT '0',
  `hd_last_time` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`hd_domain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_users`
--

CREATE TABLE IF NOT EXISTS `neltool_users` (
  `user_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(32) NOT NULL DEFAULT '',
  `user_password` varchar(64) NOT NULL DEFAULT '',
  `user_group_id` int(11) NOT NULL DEFAULT '0',
  `user_created` int(11) NOT NULL DEFAULT '0',
  `user_active` int(11) NOT NULL DEFAULT '0',
  `user_logged_last` int(11) NOT NULL DEFAULT '0',
  `user_logged_count` int(11) NOT NULL DEFAULT '0',
  `user_menu_style` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_login` (`user_name`),
  KEY `user_group_id` (`user_group_id`),
  KEY `user_active` (`user_active`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_user_applications`
--

CREATE TABLE IF NOT EXISTS `neltool_user_applications` (
  `user_application_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_application_user_id` int(11) NOT NULL DEFAULT '0',
  `user_application_application_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_application_id`),
  KEY `user_application_user_id` (`user_application_user_id`),
  KEY `user_application_application_id` (`user_application_application_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_user_domains`
--

CREATE TABLE IF NOT EXISTS `neltool_user_domains` (
  `user_domain_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_domain_user_id` int(11) NOT NULL DEFAULT '0',
  `user_domain_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_domain_id`),
  KEY `user_domain_user_id` (`user_domain_user_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `neltool_user_shards`
--

CREATE TABLE IF NOT EXISTS `neltool_user_shards` (
  `user_shard_id` int(11) NOT NULL AUTO_INCREMENT,
  `user_shard_user_id` int(11) NOT NULL DEFAULT '0',
  `user_shard_shard_id` int(11) NOT NULL DEFAULT '0',
  `user_shard_domain_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_shard_id`),
  KEY `user_shard_user_id` (`user_shard_user_id`),
  KEY `user_shard_domain_id` (`user_shard_domain_id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
