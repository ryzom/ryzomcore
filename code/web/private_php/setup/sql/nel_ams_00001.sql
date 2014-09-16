-- phpMyAdmin SQL Dump
-- version 4.2.8
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Sep 02, 2014 at 03:37 PM
-- Server version: 5.5.38-0+wheezy1-log
-- PHP Version: 5.4.4-14+deb7u12

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `ams_web`
--

-- --------------------------------------------------------

--
-- Table structure for table `ams_user`
--

CREATE TABLE IF NOT EXISTS `ams_user` (
`UId` int(10) NOT NULL,
  `Login` varchar(64) NOT NULL DEFAULT '',
  `Password` varchar(13) DEFAULT NULL,
  `Email` varchar(255) NOT NULL DEFAULT '',
  `Permission` int(3) NOT NULL DEFAULT '1',
  `FirstName` varchar(255) NOT NULL DEFAULT '',
  `LastName` varchar(255) NOT NULL DEFAULT '',
  `Gender` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `Country` char(2) NOT NULL DEFAULT '',
  `ReceiveMail` int(1) NOT NULL DEFAULT '1',
  `Language` varchar(3) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `ams_user`
--
ALTER TABLE `ams_user`
 ADD PRIMARY KEY (`UId`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `ams_user`
--
ALTER TABLE `ams_user`
MODIFY `UId` int(10) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

