-- phpMyAdmin SQL Dump
-- version 4.8.5
-- https://www.phpmyadmin.net/
--
-- Hôte : localhost:3306
-- Généré le :  jeu. 21 fév. 2019 à 23:47
-- Version du serveur :  5.7.25-0ubuntu0.18.04.2
-- Version de PHP :  7.2.15-0ubuntu0.18.04.1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Base de données :  `ring_test`
--

-- --------------------------------------------------------

--
-- Structure de la table `characters`
--

CREATE TABLE `characters` (
  `char_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `char_name` varchar(20) NOT NULL DEFAULT '',
  `user_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `guild_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `best_combat_level` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `home_mainland_session_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `ring_access` varchar(63) NOT NULL DEFAULT '',
  `race` enum('r_fyros','r_matis','r_tryker','r_zorai') NOT NULL DEFAULT 'r_fyros',
  `civilisation` enum('c_neutral','c_fyros','c_fyros','c_matis','c_tryker','c_zorai') NOT NULL DEFAULT 'c_neutral',
  `cult` enum('c_neutral','c_kami','c_karavan') NOT NULL DEFAULT 'c_neutral',
  `current_session` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `rrp_am` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `rrp_masterless` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `rrp_author` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `newcomer` tinyint(1) NOT NULL DEFAULT '1',
  `creation_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `last_played_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `folder`
--

CREATE TABLE `folder` (
  `Id` int(10) UNSIGNED NOT NULL,
  `owner` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `title` varchar(40) NOT NULL DEFAULT '',
  `comments` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `folder_access`
--

CREATE TABLE `folder_access` (
  `Id` int(10) UNSIGNED NOT NULL,
  `folder_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `user_id` int(10) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `guilds`
--

CREATE TABLE `guilds` (
  `guild_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `guild_name` varchar(20) NOT NULL DEFAULT '',
  `shard_id` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `guild_invites`
--

CREATE TABLE `guild_invites` (
  `Id` int(10) UNSIGNED NOT NULL,
  `session_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `guild_id` int(10) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `journal_entry`
--

CREATE TABLE `journal_entry` (
  `Id` int(10) UNSIGNED NOT NULL,
  `session_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `author` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `type` enum('jet_credits','jet_notes') NOT NULL DEFAULT 'jet_notes',
  `text` text NOT NULL,
  `time_stamp` datetime NOT NULL DEFAULT '2005-09-07 12:41:33'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `known_users`
--

CREATE TABLE `known_users` (
  `Id` int(10) UNSIGNED NOT NULL,
  `owner` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `targer_user` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `targer_character` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `relation_type` enum('rt_friend','rt_banned','rt_friend_dm') NOT NULL DEFAULT 'rt_friend',
  `comments` varchar(255) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `mfs_erased_mail_series`
--

CREATE TABLE `mfs_erased_mail_series` (
  `erased_char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `erased_char_name` varchar(32) NOT NULL DEFAULT '',
  `erased_series` int(11) UNSIGNED NOT NULL,
  `erase_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `mfs_guild_thread`
--

CREATE TABLE `mfs_guild_thread` (
  `thread_id` int(11) NOT NULL,
  `guild_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `topic` varchar(255) NOT NULL DEFAULT '',
  `author_name` varchar(32) NOT NULL DEFAULT '',
  `last_post_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `post_count` int(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `mfs_guild_thread_message`
--

CREATE TABLE `mfs_guild_thread_message` (
  `id` int(11) NOT NULL,
  `thread_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `author_name` varchar(32) NOT NULL DEFAULT '',
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `content` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `mfs_mail`
--

CREATE TABLE `mfs_mail` (
  `id` int(11) NOT NULL,
  `sender_name` varchar(32) NOT NULL DEFAULT '',
  `subject` varchar(250) NOT NULL DEFAULT '',
  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `status` enum('ms_new','ms_read','ms_erased') NOT NULL DEFAULT 'ms_new',
  `dest_char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `erase_series` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `content` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `player_rating`
--

CREATE TABLE `player_rating` (
  `Id` int(10) UNSIGNED NOT NULL,
  `session_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `author` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `rating` int(10) NOT NULL DEFAULT '0',
  `comments` text NOT NULL,
  `time_stamp` datetime NOT NULL DEFAULT '2005-09-07 12:41:33'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `ring_users`
--

CREATE TABLE `ring_users` (
  `user_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `user_name` varchar(20) NOT NULL DEFAULT '',
  `user_type` enum('ut_character','ut_pioneer') NOT NULL DEFAULT 'ut_character',
  `current_char` varchar(255) NOT NULL DEFAULT '',
  `current_session` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `current_activity` enum('ca_none','ca_play','ca_edit','ca_anim') NOT NULL DEFAULT 'ca_none',
  `current_status` enum('cs_offline','cs_logged','cs_online') NOT NULL DEFAULT 'cs_offline',
  `public_level` varchar(255) NOT NULL DEFAULT 'pl_none',
  `account_type` enum('at_normal','at_gold') NOT NULL DEFAULT 'at_normal',
  `content_access_level` varchar(20) NOT NULL DEFAULT '',
  `description` text,
  `lang` enum('lang_en','lang_fr','lang_de') NOT NULL DEFAULT 'lang_en',
  `cookie` varchar(30) NOT NULL DEFAULT '',
  `current_domain_id` int(10) NOT NULL DEFAULT '-1',
  `pioneer_char_id` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `add_privileges` varchar(255) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `scenario_desc`
--

CREATE TABLE `scenario_desc` (
  `session_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `parent_scenario` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `description` text NOT NULL,
  `relation_to_parent` enum('rtp_same','rtp_variant','rtp_different') NOT NULL DEFAULT 'rtp_same',
  `title` varchar(40) NOT NULL DEFAULT '',
  `num_player` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `content_access_level` varchar(20) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `sessions`
--

CREATE TABLE `sessions` (
  `session_id` int(10) UNSIGNED NOT NULL,
  `session_type` enum('st_edit','st_anim','st_outland','st_mainland') NOT NULL DEFAULT 'st_edit',
  `title` varchar(40) NOT NULL DEFAULT '',
  `owner` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `plan_date` datetime NOT NULL DEFAULT '2005-09-21 12:41:33',
  `start_date` datetime NOT NULL DEFAULT '2005-08-31 00:00:00',
  `description` text NOT NULL,
  `orientation` enum('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') NOT NULL DEFAULT 'so_other',
  `level` enum('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') NOT NULL DEFAULT 'sl_a',
  `rule_type` enum('rt_strict','rt_liberal') NOT NULL DEFAULT 'rt_strict',
  `access_type` enum('at_public','at_private') NOT NULL DEFAULT 'at_private',
  `state` enum('ss_planned','ss_open','ss_locked','ss_closed') NOT NULL DEFAULT 'ss_planned',
  `host_shard_id` int(11) NOT NULL DEFAULT '0',
  `subscription_slots` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `reserved_slots` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `free_slots` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `estimated_duration` enum('et_short','et_medium','et_long') NOT NULL DEFAULT 'et_short',
  `final_duration` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `folder_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `lang` varchar(20) NOT NULL DEFAULT '',
  `icone` varchar(70) NOT NULL DEFAULT '',
  `anim_mode` enum('am_dm','am_autonomous') NOT NULL DEFAULT 'am_dm',
  `race_filter` set('rf_fyros','rf_matis','rf_tryker','rf_zorai') NOT NULL DEFAULT '',
  `religion_filter` set('rf_kami','rf_karavan','rf_neutral') NOT NULL DEFAULT '',
  `guild_filter` enum('gf_only_my_guild','gf_any_player') DEFAULT 'gf_only_my_guild',
  `shard_filter` set('sf_shard00','sf_shard01','sf_shard02','sf_shard03','sf_shard04','sf_shard05','sf_shard06','sf_shard07','sf_shard08','sf_shard09','sf_shard10','sf_shard11','sf_shard12','sf_shard13','sf_shard14','sf_shard15','sf_shard16','sf_shard17','sf_shard18','sf_shard19','sf_shard20','sf_shard21','sf_shard22','sf_shard23','sf_shard24','sf_shard25','sf_shard26','sf_shard27','sf_shard28','sf_shard29','sf_shard30','sf_shard31') NOT NULL DEFAULT 'sf_shard00,sf_shard01,sf_shard02,sf_shard03,sf_shard04,sf_shard05,sf_shard06,sf_shard07,sf_shard08,sf_shard09,sf_shard10,sf_shard11,sf_shard12,sf_shard13,sf_shard14,sf_shard15,sf_shard16,sf_shard17,sf_shard18,sf_shard19,sf_shard20,sf_shard21,sf_shard22,sf_shard23,sf_shard24,sf_shard25,sf_shard26,sf_shard27,sf_shard28,sf_shard29,sf_shard30,sf_shard31',
  `level_filter` set('lf_a','lf_b','lf_c','lf_d','lf_e','lf_f') NOT NULL DEFAULT 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f',
  `subscription_closed` tinyint(1) NOT NULL DEFAULT '0',
  `newcomer` tinyint(1) UNSIGNED ZEROFILL NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Structure de la table `session_participant`
--

CREATE TABLE `session_participant` (
  `Id` int(10) UNSIGNED NOT NULL,
  `session_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `status` enum('sps_play_subscribed','sps_play_invited','sps_edit_invited','sps_anim_invited','sps_playing','sps_editing','sps_animating') NOT NULL DEFAULT 'sps_play_subscribed',
  `kicked` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `session_rated` tinyint(1) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `shard`
--

CREATE TABLE `shard` (
  `shard_id` int(10) NOT NULL DEFAULT '0',
  `WSOnline` tinyint(1) NOT NULL DEFAULT '0',
  `MOTD` text NOT NULL,
  `OldState` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_restricted',
  `RequiredState` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

--
-- Index pour les tables déchargées
--

--
-- Index pour la table `characters`
--
ALTER TABLE `characters`
  ADD PRIMARY KEY (`char_id`),
  ADD UNIQUE KEY `char_name_idx` (`char_name`,`home_mainland_session_id`),
  ADD KEY `user_id_idx` (`user_id`),
  ADD KEY `guild_idx` (`guild_id`),
  ADD KEY `guild_id_idx` (`guild_id`);

--
-- Index pour la table `folder`
--
ALTER TABLE `folder`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `owner_idx` (`owner`),
  ADD KEY `title_idx` (`title`);

--
-- Index pour la table `folder_access`
--
ALTER TABLE `folder_access`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `folder_id_idx` (`folder_id`),
  ADD KEY `user_idx` (`user_id`);

--
-- Index pour la table `guilds`
--
ALTER TABLE `guilds`
  ADD PRIMARY KEY (`guild_id`),
  ADD UNIQUE KEY `huild_name_idx` (`guild_name`),
  ADD KEY `shard_id_idx` (`shard_id`);

--
-- Index pour la table `guild_invites`
--
ALTER TABLE `guild_invites`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `guild_id_idx` (`guild_id`),
  ADD KEY `session_id_idx` (`session_id`);

--
-- Index pour la table `journal_entry`
--
ALTER TABLE `journal_entry`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `session_id_idx` (`session_id`);

--
-- Index pour la table `known_users`
--
ALTER TABLE `known_users`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `user_index` (`owner`);

--
-- Index pour la table `mfs_erased_mail_series`
--
ALTER TABLE `mfs_erased_mail_series`
  ADD PRIMARY KEY (`erased_series`);

--
-- Index pour la table `mfs_guild_thread`
--
ALTER TABLE `mfs_guild_thread`
  ADD PRIMARY KEY (`thread_id`),
  ADD KEY `guild_index` (`guild_id`);

--
-- Index pour la table `mfs_guild_thread_message`
--
ALTER TABLE `mfs_guild_thread_message`
  ADD PRIMARY KEY (`id`);

--
-- Index pour la table `mfs_mail`
--
ALTER TABLE `mfs_mail`
  ADD PRIMARY KEY (`id`),
  ADD KEY `dest_index` (`dest_char_id`);

--
-- Index pour la table `player_rating`
--
ALTER TABLE `player_rating`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `session_id_idx` (`session_id`),
  ADD KEY `author_idx` (`author`);

--
-- Index pour la table `ring_users`
--
ALTER TABLE `ring_users`
  ADD PRIMARY KEY (`user_id`),
  ADD UNIQUE KEY `user_name_idx` (`user_name`),
  ADD KEY `cookie_idx` (`cookie`);

--
-- Index pour la table `scenario_desc`
--
ALTER TABLE `scenario_desc`
  ADD PRIMARY KEY (`session_id`),
  ADD UNIQUE KEY `title_idx` (`title`),
  ADD KEY `parent_idx` (`parent_scenario`);

--
-- Index pour la table `sessions`
--
ALTER TABLE `sessions`
  ADD PRIMARY KEY (`session_id`),
  ADD KEY `owner_idx` (`owner`),
  ADD KEY `folder_idx` (`folder_id`),
  ADD KEY `state_type_idx` (`state`,`session_type`);

--
-- Index pour la table `session_participant`
--
ALTER TABLE `session_participant`
  ADD PRIMARY KEY (`Id`),
  ADD KEY `session_idx` (`session_id`),
  ADD KEY `user_idx` (`char_id`);

--
-- Index pour la table `shard`
--
ALTER TABLE `shard`
  ADD PRIMARY KEY (`shard_id`);

--
-- AUTO_INCREMENT pour les tables déchargées
--

--
-- AUTO_INCREMENT pour la table `folder`
--
ALTER TABLE `folder`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `folder_access`
--
ALTER TABLE `folder_access`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `guild_invites`
--
ALTER TABLE `guild_invites`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `journal_entry`
--
ALTER TABLE `journal_entry`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `known_users`
--
ALTER TABLE `known_users`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `mfs_erased_mail_series`
--
ALTER TABLE `mfs_erased_mail_series`
  MODIFY `erased_series` int(11) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `mfs_guild_thread`
--
ALTER TABLE `mfs_guild_thread`
  MODIFY `thread_id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `mfs_guild_thread_message`
--
ALTER TABLE `mfs_guild_thread_message`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `mfs_mail`
--
ALTER TABLE `mfs_mail`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `player_rating`
--
ALTER TABLE `player_rating`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `sessions`
--
ALTER TABLE `sessions`
  MODIFY `session_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- AUTO_INCREMENT pour la table `session_participant`
--
ALTER TABLE `session_participant`
  MODIFY `Id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
