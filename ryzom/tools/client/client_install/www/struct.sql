-- phpMyAdmin SQL Dump
-- version 2.6.1
-- http://www.phpmyadmin.net
-- 
-- Serveur: localhost
-- Généré le : Vendredi 09 Mars 2007 à 15:00
-- Version du serveur: 4.1.9
-- Version de PHP: 4.3.10
-- 
-- Base de données: `stats`
-- 

-- --------------------------------------------------------

-- 
-- Structure de la table `install_users`
-- 

DROP TABLE IF EXISTS `install_users`;
CREATE TABLE IF NOT EXISTS `install_users` (
  `user_id` int(11) NOT NULL default '0',
  `install_id` int(11) NOT NULL default '0',
  `first_install` datetime NOT NULL default '0000-00-00 00:00:00',
  `last_install` datetime NOT NULL default '0000-00-00 00:00:00',
  `install_count` tinyint(4) NOT NULL default '0',
  `os` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `proc` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `memory` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `video_card` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `driver_version` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `state` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  PRIMARY KEY  (`user_id`),
  KEY `install_id` (`install_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

-- 
-- Structure de la table `log`
-- 

DROP TABLE IF EXISTS `log`;
CREATE TABLE IF NOT EXISTS `log` (
  `log_id` int(11) NOT NULL auto_increment,
  `log` text character set latin1 collate latin1_general_cs NOT NULL,
  PRIMARY KEY  (`log_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=687 ;

-- --------------------------------------------------------

-- 
-- Structure de la table `sessions`
-- 

DROP TABLE IF EXISTS `sessions`;
CREATE TABLE IF NOT EXISTS `sessions` (
  `session_id` int(11) NOT NULL default '0',
  `user_id` int(11) NOT NULL default '0',
  `server` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `application` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `version` int(11) NOT NULL default '0',
  `ip` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `lang` varchar(5) character set latin1 collate latin1_general_cs NOT NULL default '',
  `type` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `package` varchar(10) character set latin1 collate latin1_general_cs NOT NULL default '',
  `protocol` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `size_download` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `size_install` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  `start_download` datetime NOT NULL default '0000-00-00 00:00:00',
  `stop_download` datetime NOT NULL default '0000-00-00 00:00:00',
  `start_install` datetime NOT NULL default '0000-00-00 00:00:00',
  `stop_install` timestamp NOT NULL default '0000-00-00 00:00:00',
  `percent_download` int(11) NOT NULL default '0',
  `percent_install` int(11) NOT NULL default '0',
  `previous_download` tinytext character set latin1 collate latin1_general_cs NOT NULL,
  PRIMARY KEY  (`session_id`),
  KEY `start_download` (`start_download`),
  KEY `user_id` (`user_id`),
  KEY `percent_download` (`percent_download`),
  KEY `percent_install` (`percent_install`),
  KEY `lang` (`lang`),
  KEY `package` (`package`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
        

