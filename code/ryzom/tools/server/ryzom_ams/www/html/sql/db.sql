CREATE DATABASE IF NOT EXISTS `ryzom_ams`;
USE `ryzom_ams`;
DROP TABLE IF EXISTS ams_user;
DROP TABLE IF EXISTS ams_querycache;

CREATE TABLE IF NOT EXISTS `ams_user` (
  `UId` int(10) NOT NULL AUTO_INCREMENT,
  `Login` varchar(64) NOT NULL DEFAULT '',
  `Password` varchar(13) DEFAULT NULL,
  `Email` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`UId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users information for ryzom_ams';

CREATE TABLE ams_querycache (
    `SID` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,
    `type` VARCHAR( 64 ) NOT NULL ,
    `query` VARCHAR( 512 ) NOT NULL 
);