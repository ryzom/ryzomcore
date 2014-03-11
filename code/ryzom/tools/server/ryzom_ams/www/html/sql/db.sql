CREATE DATABASE IF NOT EXISTS `ryzom_ams`;
USE `ryzom_ams`;
DROP TABLE IF EXISTS ams_user;

CREATE TABLE IF NOT EXISTS `ams_user` (
  `UId` int(10) NOT NULL AUTO_INCREMENT,
  `Login` varchar(64) NOT NULL DEFAULT '',
  `Password` varchar(13) DEFAULT NULL,
  `Email` varchar(255) NOT NULL DEFAULT '',
  `Permission` int(3) NOT NULL DEFAULT 1,
  PRIMARY KEY (`UId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users information for ryzom_ams';

);

CREATE DATABASE IF NOT EXISTS `ryzom_ams_lib`;
USE `ryzom_ams_lib`;
DROP TABLE IF EXISTS ams_querycache;

CREATE TABLE ams_querycache (
    `SID` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,
    `type` VARCHAR( 64 ) NOT NULL ,
    `query` VARCHAR( 512 ) NOT NULL 
);