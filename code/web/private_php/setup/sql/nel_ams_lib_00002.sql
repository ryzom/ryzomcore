-- phpMyAdmin SQL Dump
-- version 4.2.8
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Sep 04, 2014 at 09:28 PM
-- Server version: 5.6.17
-- PHP Version: 5.5.12

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
-- Table structure for table `ticket_attachments`
--

CREATE TABLE IF NOT EXISTS `ticket_attachments` (
`idticket_attachments` int(10) unsigned NOT NULL,
  `ticket_TId` int(10) unsigned NOT NULL,
  `Filename` varchar(45) COLLATE utf8_unicode_ci NOT NULL,
  `Timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `Filesize` int(10) NOT NULL,
  `Uploader` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `ticket_attachments`
--
ALTER TABLE `ticket_attachments`
 ADD PRIMARY KEY (`idticket_attachments`), ADD UNIQUE KEY `idticket_attachments_UNIQUE` (`idticket_attachments`), ADD KEY `fk_ticket_attachments_ticket1_idx` (`ticket_TId`), ADD KEY `fk_ticket_attachments_ticket_user1_idx` (`Uploader`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `ticket_attachments`
--
ALTER TABLE `ticket_attachments`
MODIFY `idticket_attachments` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- Constraints for dumped tables
--

--
-- Constraints for table `ticket_attachments`
--
ALTER TABLE `ticket_attachments`
ADD CONSTRAINT `fk_ticket_attachments_ticket1` FOREIGN KEY (`ticket_TId`) REFERENCES `ticket` (`TId`) ON DELETE NO ACTION ON UPDATE NO ACTION,
ADD CONSTRAINT `fk_ticket_attachments_ticket_user1` FOREIGN KEY (`Uploader`) REFERENCES `ticket_user` (`TUserId`) ON DELETE NO ACTION ON UPDATE NO ACTION;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
