-- phpMyAdmin SQL Dump
-- version 4.2.8
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Sep 02, 2014 at 04:12 PM
-- Server version: 5.5.38-0+wheezy1-log
-- PHP Version: 5.4.4-14+deb7u12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `nel_ams_lib`
--

-- --------------------------------------------------------

--
-- Table structure for table `ams_querycache`
--

CREATE TABLE IF NOT EXISTS `ams_querycache` (
`SID` int(11) NOT NULL,
  `type` varchar(64) NOT NULL,
  `query` varchar(512) NOT NULL,
  `db` varchar(80) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `assigned`
--

CREATE TABLE IF NOT EXISTS `assigned` (
  `Ticket` int(10) unsigned NOT NULL,
  `User` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `email`
--

CREATE TABLE IF NOT EXISTS `email` (
`MailId` int(11) NOT NULL,
  `Recipient` varchar(50) DEFAULT NULL,
  `Subject` varchar(60) DEFAULT NULL,
  `Body` varchar(400) DEFAULT NULL,
  `Status` varchar(45) DEFAULT NULL,
  `Attempts` varchar(45) DEFAULT '0',
  `UserId` int(10) unsigned DEFAULT NULL,
  `MessageId` varchar(45) DEFAULT NULL,
  `TicketId` int(10) unsigned DEFAULT NULL,
  `Sender` int(10) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `forwarded`
--

CREATE TABLE IF NOT EXISTS `forwarded` (
  `Group` int(10) unsigned NOT NULL,
  `Ticket` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `in_group`
--

CREATE TABLE IF NOT EXISTS `in_group` (
  `Ticket_Group` int(10) unsigned NOT NULL,
  `Ticket` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `in_support_group`
--

CREATE TABLE IF NOT EXISTS `in_support_group` (
  `User` int(10) unsigned NOT NULL,
  `Group` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `plugins`
--

CREATE TABLE IF NOT EXISTS `plugins` (
`Id` int(10) NOT NULL,
  `FileName` varchar(255) NOT NULL,
  `Name` varchar(56) NOT NULL,
  `Type` varchar(12) NOT NULL,
  `Owner` varchar(25) NOT NULL,
  `Permission` varchar(5) NOT NULL,
  `Status` int(11) NOT NULL DEFAULT '0',
  `Weight` int(11) NOT NULL DEFAULT '0',
  `Info` text
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `plugins`
--

INSERT INTO `plugins` (`Id`, `FileName`, `Name`, `Type`, `Owner`, `Permission`, `Status`, `Weight`, `Info`) VALUES
(1, 'API_key_management', 'API_key_management', 'automatic', '', 'admin', 0, 0, '{"PluginName":"API Key Management","Description":"Provides public access to the API''s by generating access tokens.","Version":"1.0.0","Type":"automatic","TemplatePath":"..\\/..\\/..\\/private_php\\/ams\\/plugins\\/API_key_management\\/templates\\/index.tpl","":null}'),
(2, 'Achievements', 'Achievements', 'Manual', '', 'admin', 0, 0, '{"PluginName":"Achievements","Description":"Returns the achivements of a user with respect to the character =.","Version":"1.0.0","TemplatePath":"..\\/..\\/..\\/private_php\\/ams\\/plugins\\/Achievements\\/templates\\/index.tpl","Type":"Manual","":null}');

-- --------------------------------------------------------

--
-- Table structure for table `support_group`
--

CREATE TABLE IF NOT EXISTS `support_group` (
`SGroupId` int(10) unsigned NOT NULL,
  `Name` varchar(22) NOT NULL,
  `Tag` varchar(7) NOT NULL,
  `GroupEmail` varchar(45) DEFAULT NULL,
  `IMAP_MailServer` varchar(60) DEFAULT NULL,
  `IMAP_Username` varchar(45) DEFAULT NULL,
  `IMAP_Password` varchar(90) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tag`
--

CREATE TABLE IF NOT EXISTS `tag` (
`TagId` int(10) unsigned NOT NULL,
  `Value` varchar(60) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tagged`
--

CREATE TABLE IF NOT EXISTS `tagged` (
  `Ticket` int(10) unsigned NOT NULL,
  `Tag` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket`
--

CREATE TABLE IF NOT EXISTS `ticket` (
`TId` int(10) unsigned NOT NULL,
  `Timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `Title` varchar(120) NOT NULL,
  `Status` int(11) DEFAULT '0',
  `Queue` int(11) DEFAULT '0',
  `Ticket_Category` int(10) unsigned NOT NULL,
  `Author` int(10) unsigned NOT NULL,
  `Priority` int(3) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket_category`
--

CREATE TABLE IF NOT EXISTS `ticket_category` (
`TCategoryId` int(10) unsigned NOT NULL,
  `Name` varchar(45) NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

--
-- Dumping data for table `ticket_category`
--

INSERT INTO `ticket_category` (`TCategoryId`, `Name`) VALUES
(2, 'Hacking'),
(3, 'Ingame-Bug'),
(5, 'Installation'),
(1, 'Uncategorized'),
(4, 'Website-Bug');

-- --------------------------------------------------------

--
-- Table structure for table `ticket_content`
--

CREATE TABLE IF NOT EXISTS `ticket_content` (
`TContentId` int(10) unsigned NOT NULL,
  `Content` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket_group`
--

CREATE TABLE IF NOT EXISTS `ticket_group` (
`TGroupId` int(10) unsigned NOT NULL,
  `Title` varchar(80) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket_info`
--

CREATE TABLE IF NOT EXISTS `ticket_info` (
`TInfoId` int(10) unsigned NOT NULL,
  `Ticket` int(10) unsigned NOT NULL,
  `ShardId` int(11) DEFAULT NULL,
  `UserPosition` varchar(65) DEFAULT NULL,
  `ViewPosition` varchar(65) DEFAULT NULL,
  `ClientVersion` varchar(65) DEFAULT NULL,
  `PatchVersion` varchar(65) DEFAULT NULL,
  `ServerTick` varchar(40) DEFAULT NULL,
  `ConnectState` varchar(40) DEFAULT NULL,
  `LocalAddress` varchar(70) DEFAULT NULL,
  `Memory` varchar(60) DEFAULT NULL,
  `OS` varchar(120) DEFAULT NULL,
  `Processor` varchar(120) DEFAULT NULL,
  `CPUID` varchar(50) DEFAULT NULL,
  `CpuMask` varchar(50) DEFAULT NULL,
  `HT` varchar(35) DEFAULT NULL,
  `NeL3D` varchar(120) DEFAULT NULL,
  `PlayerName` varchar(45) DEFAULT NULL,
  `UserId` int(11) DEFAULT NULL,
  `TimeInGame` varchar(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket_log`
--

CREATE TABLE IF NOT EXISTS `ticket_log` (
`TLogId` int(10) unsigned NOT NULL,
  `Timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `Query` varchar(255) NOT NULL,
  `Ticket` int(10) unsigned NOT NULL,
  `Author` int(10) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket_reply`
--

CREATE TABLE IF NOT EXISTS `ticket_reply` (
`TReplyId` int(10) unsigned NOT NULL,
  `Ticket` int(10) unsigned NOT NULL,
  `Author` int(10) unsigned NOT NULL,
  `Content` int(10) unsigned NOT NULL,
  `Timestamp` timestamp NULL DEFAULT NULL,
  `Hidden` tinyint(1) DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ticket_user`
--

CREATE TABLE IF NOT EXISTS `ticket_user` (
`TUserId` int(10) unsigned NOT NULL,
  `Permission` int(3) NOT NULL DEFAULT '1',
  `ExternId` int(10) unsigned NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `updates`
--

CREATE TABLE IF NOT EXISTS `updates` (
`s.no` int(10) NOT NULL,
  `PluginId` int(10) DEFAULT NULL,
  `UpdatePath` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT NULL,
  `UpdateInfo` text CHARACTER SET utf8 COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `ams_querycache`
--
ALTER TABLE `ams_querycache`
 ADD PRIMARY KEY (`SID`);

--
-- Indexes for table `assigned`
--
ALTER TABLE `assigned`
 ADD PRIMARY KEY (`Ticket`,`User`), ADD KEY `fk_assigned_ticket_idx` (`Ticket`), ADD KEY `fk_assigned_ams_user_idx` (`User`);

--
-- Indexes for table `email`
--
ALTER TABLE `email`
 ADD PRIMARY KEY (`MailId`), ADD KEY `fk_email_ticket_user2` (`UserId`), ADD KEY `fk_email_ticket1` (`TicketId`), ADD KEY `fk_email_support_group1` (`Sender`);

--
-- Indexes for table `forwarded`
--
ALTER TABLE `forwarded`
 ADD KEY `fk_forwarded_support_group1` (`Group`), ADD KEY `fk_forwarded_ticket1` (`Ticket`);

--
-- Indexes for table `in_group`
--
ALTER TABLE `in_group`
 ADD PRIMARY KEY (`Ticket_Group`,`Ticket`), ADD KEY `fk_in_group_ticket_group_idx` (`Ticket_Group`), ADD KEY `fk_in_group_ticket_idx` (`Ticket`);

--
-- Indexes for table `in_support_group`
--
ALTER TABLE `in_support_group`
 ADD KEY `fk_in_support_group_ticket_user1` (`User`), ADD KEY `fk_in_support_group_support_group1` (`Group`);

--
-- Indexes for table `plugins`
--
ALTER TABLE `plugins`
 ADD PRIMARY KEY (`Id`);

--
-- Indexes for table `support_group`
--
ALTER TABLE `support_group`
 ADD PRIMARY KEY (`SGroupId`), ADD UNIQUE KEY `Name_UNIQUE` (`Name`), ADD UNIQUE KEY `Tag_UNIQUE` (`Tag`);

--
-- Indexes for table `tag`
--
ALTER TABLE `tag`
 ADD PRIMARY KEY (`TagId`), ADD UNIQUE KEY `Value_UNIQUE` (`Value`);

--
-- Indexes for table `tagged`
--
ALTER TABLE `tagged`
 ADD PRIMARY KEY (`Ticket`,`Tag`), ADD KEY `fk_tagged_tag_idx` (`Tag`);

--
-- Indexes for table `ticket`
--
ALTER TABLE `ticket`
 ADD PRIMARY KEY (`TId`), ADD KEY `fk_ticket_ticket_category_idx` (`Ticket_Category`), ADD KEY `fk_ticket_ams_user_idx` (`Author`);

--
-- Indexes for table `ticket_category`
--
ALTER TABLE `ticket_category`
 ADD PRIMARY KEY (`TCategoryId`), ADD UNIQUE KEY `Name_UNIQUE` (`Name`);

--
-- Indexes for table `ticket_content`
--
ALTER TABLE `ticket_content`
 ADD PRIMARY KEY (`TContentId`);

--
-- Indexes for table `ticket_group`
--
ALTER TABLE `ticket_group`
 ADD PRIMARY KEY (`TGroupId`), ADD UNIQUE KEY `Title_UNIQUE` (`Title`);

--
-- Indexes for table `ticket_info`
--
ALTER TABLE `ticket_info`
 ADD PRIMARY KEY (`TInfoId`), ADD KEY `fk_ticket_info_ticket1` (`Ticket`);

--
-- Indexes for table `ticket_log`
--
ALTER TABLE `ticket_log`
 ADD PRIMARY KEY (`TLogId`), ADD KEY `fk_ticket_log_ticket1` (`Ticket`), ADD KEY `fk_ticket_log_ticket_user1` (`Author`);

--
-- Indexes for table `ticket_reply`
--
ALTER TABLE `ticket_reply`
 ADD PRIMARY KEY (`TReplyId`), ADD KEY `fk_ticket_reply_ticket_idx` (`Ticket`), ADD KEY `fk_ticket_reply_ams_user_idx` (`Author`), ADD KEY `fk_ticket_reply_content_idx` (`Content`);

--
-- Indexes for table `ticket_user`
--
ALTER TABLE `ticket_user`
 ADD PRIMARY KEY (`TUserId`);

--
-- Indexes for table `updates`
--
ALTER TABLE `updates`
 ADD PRIMARY KEY (`s.no`), ADD KEY `PluginId` (`PluginId`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `ams_querycache`
--
ALTER TABLE `ams_querycache`
MODIFY `SID` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `email`
--
ALTER TABLE `email`
MODIFY `MailId` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `plugins`
--
ALTER TABLE `plugins`
MODIFY `Id` int(10) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT for table `support_group`
--
ALTER TABLE `support_group`
MODIFY `SGroupId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `tag`
--
ALTER TABLE `tag`
MODIFY `TagId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket`
--
ALTER TABLE `ticket`
MODIFY `TId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket_category`
--
ALTER TABLE `ticket_category`
MODIFY `TCategoryId` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=6;
--
-- AUTO_INCREMENT for table `ticket_content`
--
ALTER TABLE `ticket_content`
MODIFY `TContentId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket_group`
--
ALTER TABLE `ticket_group`
MODIFY `TGroupId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket_info`
--
ALTER TABLE `ticket_info`
MODIFY `TInfoId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket_log`
--
ALTER TABLE `ticket_log`
MODIFY `TLogId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket_reply`
--
ALTER TABLE `ticket_reply`
MODIFY `TReplyId` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ticket_user`
--
ALTER TABLE `ticket_user`
MODIFY `TUserId` int(10) unsigned NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `updates`
--
ALTER TABLE `updates`
MODIFY `s.no` int(10) NOT NULL AUTO_INCREMENT;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `assigned`
--
ALTER TABLE `assigned`
ADD CONSTRAINT `fk_assigned_ticket` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_assigned_ams_user` FOREIGN KEY (`User`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `email`
--
ALTER TABLE `email`
ADD CONSTRAINT `fk_email_ticket_user2` FOREIGN KEY (`UserId`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_email_ticket1` FOREIGN KEY (`TicketId`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_email_support_group1` FOREIGN KEY (`Sender`) REFERENCES `support_group` (`SGroupId`) ON DELETE CASCADE ON UPDATE NO ACTION;

--
-- Constraints for table `forwarded`
--
ALTER TABLE `forwarded`
ADD CONSTRAINT `fk_forwarded_support_group1` FOREIGN KEY (`Group`) REFERENCES `support_group` (`SGroupId`) ON DELETE CASCADE ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_forwarded_ticket1` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `in_group`
--
ALTER TABLE `in_group`
ADD CONSTRAINT `fk_in_group_ticket_group` FOREIGN KEY (`Ticket_Group`) REFERENCES `ticket_group` (`TGroupId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_in_group_ticket` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `in_support_group`
--
ALTER TABLE `in_support_group`
ADD CONSTRAINT `fk_in_support_group_ticket_user1` FOREIGN KEY (`User`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_in_support_group_support_group1` FOREIGN KEY (`Group`) REFERENCES `support_group` (`SGroupId`) ON DELETE CASCADE ON UPDATE NO ACTION;

--
-- Constraints for table `tagged`
--
ALTER TABLE `tagged`
ADD CONSTRAINT `fk_tagged_ticket` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_tagged_tag` FOREIGN KEY (`Tag`) REFERENCES `tag` (`TagId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `ticket`
--
ALTER TABLE `ticket`
ADD CONSTRAINT `fk_ticket_ticket_category` FOREIGN KEY (`Ticket_Category`) REFERENCES `ticket_category` (`TCategoryId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_ticket_ams_user` FOREIGN KEY (`Author`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `ticket_info`
--
ALTER TABLE `ticket_info`
ADD CONSTRAINT `fk_ticket_info_ticket1` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `ticket_log`
--
ALTER TABLE `ticket_log`
ADD CONSTRAINT `fk_ticket_log_ticket1` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_ticket_log_ticket_user1` FOREIGN KEY (`Author`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `ticket_reply`
--
ALTER TABLE `ticket_reply`
ADD CONSTRAINT `fk_ticket_reply_ticket` FOREIGN KEY (`Ticket`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_ticket_reply_ams_user` FOREIGN KEY (`Author`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_ticket_reply_ticket_content` FOREIGN KEY (`Content`) REFERENCES `ticket_content` (`TContentId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

--
-- Constraints for table `updates`
--
ALTER TABLE `updates`
ADD CONSTRAINT `updates_ibfk_1` FOREIGN KEY (`PluginId`) REFERENCES `plugins` (`Id`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
