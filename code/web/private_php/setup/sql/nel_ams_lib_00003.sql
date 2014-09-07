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
-- Dumping data for table `plugins`
--

INSERT INTO `plugins` (`Id`, `FileName`, `Name`, `Type`, `Owner`, `Permission`, `Status`, `Weight`, `Info`) VALUES
(3, 'Domain_Management', 'Domain_Management', 'Manual', '', 'admin', 1, 0, '{"PluginName":"Domain Management","Description":"Plug-in for Domain Management.","Version":"1.0.0","TemplatePath":"..\\/..\\/..\\/private_php\\/ams\\/plugins\\/Domain_Management\\/templates\\/index.tpl","Type":"Manual","":null}');

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
