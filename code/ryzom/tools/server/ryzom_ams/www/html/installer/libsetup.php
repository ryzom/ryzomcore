<?php

    /**
    * This script will install all databases related to the Ryzom AMS and it will generate an admin account..
    * @author Daan Janssens, mentored by Matthew Lagoe
    */
    
    if (!isset($_POST['function'])) { 
        //require the pages that are being needed.
        require_once( '../config.default.php' );
        require_once( '../../ams_lib/libinclude.php' );
        ini_set( "display_errors", true );
        error_reporting( E_ALL );

        $return = array();
            
        helpers :: loadTemplate( "install" , $return );
        exit;
    } else {
    
        if (file_exists('../config.php')) {
            require( '../config.php' );
        } else {
            require( '../config.default.php' );
        }

        //var used to access the DB;
        global $cfg;
        
        sleep(15);
        try{
            //SETUP THE WWW DB
            $dbw = new DBLayer("install", "web");
            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['web']['name'] ."`;
                USE `". $cfg['db']['web']['name'] . "`;
                DROP TABLE IF EXISTS ams_user;
                
                CREATE TABLE IF NOT EXISTS `ams_user` (
                `UId` int(10) NOT NULL AUTO_INCREMENT,
                `Login` varchar(64) NOT NULL DEFAULT '',
                `Password` varchar(13) DEFAULT NULL,
                `Email` varchar(255) NOT NULL DEFAULT '',
                `Permission` int(3) NOT NULL DEFAULT 1,
                `FirstName` varchar(255) NOT NULL DEFAULT '',
                `LastName` varchar(255) NOT NULL DEFAULT '',
                `Gender` tinyint(1) unsigned NOT NULL DEFAULT '0',
                `Country` char(2) NOT NULL DEFAULT '',
                `ReceiveMail` int(1) NOT NULL DEFAULT 1,
                `Language` varchar(3) DEFAULT NULL,
                PRIMARY KEY (`UId`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users information for ryzom_ams';
                
                );     

                GRANT ALL ON `" . $cfg['db']['web']['name'] ."`.* TO `" . $cfg['db']['web']['user'] ."`@localhost;
            ";
            $dbw->executeWithoutParams($sql);
            
            //SETUP THE AMS_LIB DB
            $dbl = new DBLayer("install", "lib");

            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`;
                USE `" . $cfg['db']['lib']['name'] ."`;
                DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ams_querycache`;
                
                CREATE TABLE ams_querycache (
                `SID` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,
                `type` VARCHAR( 64 ) NOT NULL ,
                `query` VARCHAR( 512 ) NOT NULL,
                `db` VARCHAR( 80 ) NOT NULL
                );
                
            -- -----------------------------------------------------------------------------------------------------------------------
            -- -----------------------------------------------------------------------------------------------------------------------

            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_log` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`tagged` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`tag` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`in_support_group` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`in_group` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_group` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_info` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`email` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`forwarded` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`assigned` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_reply` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_content` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`support_group` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_category` ;
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_user` ;

            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_category`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_category` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_category` (
              `TCategoryId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Name` VARCHAR(45) NOT NULL ,
              PRIMARY KEY (`TCategoryId`) ,
              UNIQUE INDEX `Name_UNIQUE` (`Name` ASC) )
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_user`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_user` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (
              `TUserId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Permission` INT(3) NOT NULL DEFAULT 1 ,
              `ExternId` INT(10) UNSIGNED NOT NULL ,
              PRIMARY KEY (`TUserId`) )
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket` (
              `TId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Timestamp` TIMESTAMP NOT NULL ,
              `Title` VARCHAR(120) NOT NULL ,
              `Status` INT NULL DEFAULT 0 ,
              `Queue` INT NULL DEFAULT 0 ,
              `Ticket_Category` INT UNSIGNED NOT NULL ,
              `Author` INT UNSIGNED NOT NULL ,
              `Priority` INT(3) NULL DEFAULT 0 ,
              PRIMARY KEY (`TId`) ,
              INDEX `fk_ticket_ticket_category_idx` (`Ticket_Category` ASC) ,
              INDEX `fk_ticket_ams_user_idx` (`Author` ASC) ,
              CONSTRAINT `fk_ticket_ticket_category`
                FOREIGN KEY (`Ticket_Category` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_category` (`TCategoryId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_ticket_ams_user`
                FOREIGN KEY (`Author` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`assigned`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`assigned` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`assigned` (
              `Ticket` INT UNSIGNED NOT NULL ,
              `User` INT UNSIGNED NOT NULL ,
              INDEX `fk_assigned_ticket_idx` (`Ticket` ASC) ,
              PRIMARY KEY (`Ticket`, `User`) ,
              INDEX `fk_assigned_ams_user_idx` (`User` ASC) ,
              CONSTRAINT `fk_assigned_ticket`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_assigned_ams_user`
                FOREIGN KEY (`User` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`tag`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`tag` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`tag` (
              `TagId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Value` VARCHAR(60) NOT NULL ,
              PRIMARY KEY (`TagId`) ,
              UNIQUE INDEX `Value_UNIQUE` (`Value` ASC) )
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`tagged`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`tagged` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`tagged` (
              `Ticket` INT UNSIGNED NOT NULL ,
              `Tag` INT UNSIGNED NOT NULL ,
              PRIMARY KEY (`Ticket`, `Tag`) ,
              INDEX `fk_tagged_tag_idx` (`Tag` ASC) ,
              CONSTRAINT `fk_tagged_ticket`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_tagged_tag`
                FOREIGN KEY (`Tag` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`tag` (`TagId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_content`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_content` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_content` (
              `TContentId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Content` TEXT NULL ,
              PRIMARY KEY (`TContentId`) )
            ENGINE = InnoDB
            DEFAULT CHARACTER SET = utf8;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_reply`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_reply` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_reply` (
              `TReplyId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Ticket` INT UNSIGNED NOT NULL ,
              `Author` INT UNSIGNED NOT NULL ,
              `Content` INT UNSIGNED NOT NULL ,
              `Timestamp` TIMESTAMP NULL ,
              `Hidden` TINYINT(1) NULL DEFAULT 0 ,
              PRIMARY KEY (`TReplyId`) ,
              INDEX `fk_ticket_reply_ticket_idx` (`Ticket` ASC) ,
              INDEX `fk_ticket_reply_ams_user_idx` (`Author` ASC) ,
              INDEX `fk_ticket_reply_content_idx` (`Content` ASC) ,
              CONSTRAINT `fk_ticket_reply_ticket`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_ticket_reply_ams_user`
                FOREIGN KEY (`Author` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_ticket_reply_ticket_content`
                FOREIGN KEY (`Content` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_content` (`TContentId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_group`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_group` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_group` (
              `TGroupId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Title` VARCHAR(80) NOT NULL ,
              PRIMARY KEY (`TGroupId`) ,
              UNIQUE INDEX `Title_UNIQUE` (`Title` ASC) )
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`in_group`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`in_group` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`in_group` (
              `Ticket_Group` INT UNSIGNED NOT NULL ,
              `Ticket` INT UNSIGNED NOT NULL ,
              PRIMARY KEY (`Ticket_Group`, `Ticket`) ,
              INDEX `fk_in_group_ticket_group_idx` (`Ticket_Group` ASC) ,
              INDEX `fk_in_group_ticket_idx` (`Ticket` ASC) ,
              CONSTRAINT `fk_in_group_ticket_group`
                FOREIGN KEY (`Ticket_Group` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_group` (`TGroupId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_in_group_ticket`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_log`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_log` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_log` (
              `TLogId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Timestamp` TIMESTAMP NOT NULL ,
              `Query` VARCHAR(255) NOT NULL ,
              `Ticket` INT UNSIGNED NOT NULL ,
              `Author` INT(10) UNSIGNED NULL ,
              PRIMARY KEY (`TLogId`) ,
              INDEX `fk_ticket_log_ticket1` (`Ticket` ASC) ,
              INDEX `fk_ticket_log_ticket_user1` (`Author` ASC) ,
              CONSTRAINT `fk_ticket_log_ticket1`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_ticket_log_ticket_user1`
                FOREIGN KEY (`Author` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
                
            INSERT IGNORE INTO `" . $cfg['db']['lib']['name'] ."`.`ticket_category` (`Name`) VALUES ('Uncategorized'),('Hacking'),('Ingame-Bug'),('Website-Bug'),('Installation');
            
                            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`support_group`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`support_group` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`support_group` (
              `SGroupId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Name` VARCHAR(22) NOT NULL ,
              `Tag` VARCHAR(7) NOT NULL ,
              `GroupEmail` VARCHAR(45) NULL ,
              `IMAP_MailServer` VARCHAR(60) NULL ,
              `IMAP_Username` VARCHAR(45) NULL ,
              `IMAP_Password` VARCHAR(90) NULL ,
              PRIMARY KEY (`SGroupId`) ,
              UNIQUE INDEX `Name_UNIQUE` (`Name` ASC) ,
              UNIQUE INDEX `Tag_UNIQUE` (`Tag` ASC) )
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`in_support_group`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`in_support_group` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`in_support_group` (
              `User` INT(10) UNSIGNED NOT NULL ,
              `Group` INT(10) UNSIGNED NOT NULL ,
              INDEX `fk_in_support_group_ticket_user1` (`User` ASC) ,
              INDEX `fk_in_support_group_support_group1` (`Group` ASC) ,
              CONSTRAINT `fk_in_support_group_ticket_user1`
                FOREIGN KEY (`User` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_in_support_group_support_group1`
                FOREIGN KEY (`Group` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`support_group` (`SGroupId` )
                ON DELETE CASCADE
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`forwarded`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`forwarded` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`forwarded` (
              `Group` INT(10) UNSIGNED NOT NULL ,
              `Ticket` INT UNSIGNED NOT NULL ,
              INDEX `fk_forwarded_support_group1` (`Group` ASC) ,
              INDEX `fk_forwarded_ticket1` (`Ticket` ASC) ,
              CONSTRAINT `fk_forwarded_support_group1`
                FOREIGN KEY (`Group` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`support_group` (`SGroupId` )
                ON DELETE CASCADE
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_forwarded_ticket1`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`email`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`email` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`email` (
              `MailId` INT NOT NULL AUTO_INCREMENT ,
              `Recipient` VARCHAR(50) NULL ,
              `Subject` VARCHAR(60) NULL ,
              `Body` VARCHAR(400) NULL ,
              `Status` VARCHAR(45) NULL ,
              `Attempts` VARCHAR(45) NULL DEFAULT 0 ,
              `UserId` INT(10) UNSIGNED NULL ,
              `MessageId` VARCHAR(45) NULL ,
              `TicketId` INT UNSIGNED NULL ,
              `Sender` INT(10) UNSIGNED NULL ,
              PRIMARY KEY (`MailId`) ,
              INDEX `fk_email_ticket_user2` (`UserId` ASC) ,
              INDEX `fk_email_ticket1` (`TicketId` ASC) ,
              INDEX `fk_email_support_group1` (`Sender` ASC) ,
              CONSTRAINT `fk_email_ticket_user2`
                FOREIGN KEY (`UserId` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_email_ticket1`
                FOREIGN KEY (`TicketId` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION,
              CONSTRAINT `fk_email_support_group1`
                FOREIGN KEY (`Sender` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`support_group` (`SGroupId` )
                ON DELETE CASCADE
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            
            
            -- -----------------------------------------------------
            -- Table `" . $cfg['db']['lib']['name'] ."`.`ticket_info`
            -- -----------------------------------------------------
            DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_info` ;
            
            CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`ticket_info` (
              `TInfoId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
              `Ticket` INT UNSIGNED NOT NULL ,
              `ShardId` INT NULL ,
              `UserPosition` VARCHAR(65) NULL ,
              `ViewPosition` VARCHAR(65) NULL ,
              `ClientVersion` VARCHAR(65) NULL ,
              `PatchVersion` VARCHAR(65) NULL ,
              `ServerTick` VARCHAR(40) NULL ,
              `ConnectState` VARCHAR(40) NULL ,
              `LocalAddress` VARCHAR(70) NULL ,
              `Memory` VARCHAR(60) NULL ,
              `OS` VARCHAR(120) NULL ,
              `Processor` VARCHAR(120) NULL ,
              `CPUID` VARCHAR(50) NULL ,
              `CpuMask` VARCHAR(50) NULL ,
              `HT` VARCHAR(35) NULL ,
              `NeL3D` VARCHAR(120) NULL ,
              `PlayerName` VARCHAR(45) NULL ,
              `UserId` INT NULL ,
              `TimeInGame` VARCHAR(50) NULL ,
              PRIMARY KEY (`TInfoId`) ,
              INDEX `fk_ticket_info_ticket1` (`Ticket` ASC) ,
              CONSTRAINT `fk_ticket_info_ticket1`
                FOREIGN KEY (`Ticket` )
                REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket` (`TId` )
                ON DELETE NO ACTION
                ON UPDATE NO ACTION)
            ENGINE = InnoDB;
            GRANT ALL ON `" . $cfg['db']['lib']['name'] ."`.* TO `" . $cfg['db']['lib']['user'] ."`@localhost;
            ";
            $dbl->executeWithoutParams($sql);
            print "The Lib & Web database were correctly installed! <br />";
            
            //SETUP THE SHARD DB
            $dbs = new DBLayer("install", "shard");
            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['shard']['name'] ."`;
                USE `". $cfg['db']['shard']['name'] . "`;
                                
                CREATE TABLE IF NOT EXISTS `domain` (
                  `domain_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `domain_name` varchar(32) NOT NULL DEFAULT '',
                  `status` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev',
                  `patch_version` int(10) unsigned NOT NULL DEFAULT '0',
                  `backup_patch_url` varchar(255) DEFAULT NULL,
                  `patch_urls` text,
                  `login_address` varchar(255) NOT NULL DEFAULT '',
                  `session_manager_address` varchar(255) NOT NULL DEFAULT '',
                  `ring_db_name` varchar(255) NOT NULL DEFAULT '',
                  `web_host` varchar(255) NOT NULL DEFAULT '',
                  `web_host_php` varchar(255) NOT NULL DEFAULT '',
                  `description` varchar(200) DEFAULT NULL,
                  PRIMARY KEY (`domain_id`),
                  UNIQUE KEY `name_idx` (`domain_name`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=13 ;

                INSERT INTO `domain` (`domain_id`, `domain_name`, `status`, `patch_version`, `backup_patch_url`, `patch_urls`, `login_address`, `session_manager_address`, `ring_db_name`, `web_host`, `web_host_php`, `description`) VALUES
                (12, 'ryzom_open', 'ds_open', 610, 'http://127.0.0.1:23001', NULL, '127.0.0.1:49998', '127.0.0.1:49999', 'ring_open', '127.0.0.1:30000', '127.0.0.1:40916', 'Open Domain');

                CREATE TABLE IF NOT EXISTS `permission` (
                  `UId` int(10) unsigned NOT NULL DEFAULT '0',
                  `ClientApplication` char(64) NOT NULL DEFAULT 'ryzom',
                  `ShardId` int(10) NOT NULL DEFAULT '-1',
                  `AccessPrivilege` set('OPEN','DEV','RESTRICTED') NOT NULL DEFAULT 'OPEN',
                  `prim` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  PRIMARY KEY (`prim`),
                  KEY `UIDIndex` (`UId`)
                ) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

                CREATE TABLE IF NOT EXISTS `shard` (
                  `ShardId` int(10) NOT NULL DEFAULT '0',
                  `domain_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `WsAddr` varchar(64) DEFAULT NULL,
                  `NbPlayers` int(10) unsigned DEFAULT '0',
                  `Name` varchar(255) DEFAULT 'unknown shard',
                  `Online` tinyint(1) unsigned DEFAULT '0',
                  `ClientApplication` varchar(64) DEFAULT 'ryzom',
                  `Version` varchar(64) NOT NULL DEFAULT '',
                  `PatchURL` varchar(255) NOT NULL DEFAULT '',
                  `DynPatchURL` varchar(255) NOT NULL DEFAULT '',
                  `FixedSessionId` int(11) unsigned NOT NULL DEFAULT '0',
                  `State` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev',
                  `MOTD` text NOT NULL,
                  `prim` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  PRIMARY KEY (`prim`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 COMMENT='contains all shards informations for login system' AUTO_INCREMENT=31 ;

                INSERT INTO `shard` (`ShardId`, `domain_id`, `WsAddr`, `NbPlayers`, `Name`, `Online`, `ClientApplication`, `Version`, `PatchURL`, `DynPatchURL`, `FixedSessionId`, `State`, `MOTD`, `prim`) VALUES
                (302, 12, '127.0.0.1', 0, 'Open Shard', 0, 'ryzom_open', '', '', '', 0, 'ds_dev', '', 30);

                CREATE TABLE IF NOT EXISTS `user` (
                  `UId` int(10) NOT NULL AUTO_INCREMENT,
                  `Login` varchar(64) NOT NULL DEFAULT '',
                  `Password` varchar(13) DEFAULT NULL,
                  `ShardId` int(10) NOT NULL DEFAULT '-1',
                  `State` enum('Offline','Online') NOT NULL DEFAULT 'Offline',
                  `Privilege` varchar(255) NOT NULL DEFAULT '',
                  `GroupName` varchar(255) NOT NULL DEFAULT '',
                  `FirstName` varchar(255) NOT NULL DEFAULT '',
                  `LastName` varchar(255) NOT NULL DEFAULT '',
                  `Birthday` varchar(32) NOT NULL DEFAULT '',
                  `Gender` tinyint(1) unsigned NOT NULL DEFAULT '0',
                  `Country` char(2) NOT NULL DEFAULT '',
                  `Email` varchar(255) NOT NULL DEFAULT '',
                  `Address` varchar(255) NOT NULL DEFAULT '',
                  `City` varchar(100) NOT NULL DEFAULT '',
                  `PostalCode` varchar(10) NOT NULL DEFAULT '',
                  `USState` char(2) NOT NULL DEFAULT '',
                  `Chat` char(2) NOT NULL DEFAULT '0',
                  `BetaKeyId` int(10) unsigned NOT NULL DEFAULT '0',
                  `CachedCoupons` varchar(255) NOT NULL DEFAULT '',
                  `ProfileAccess` varchar(45) DEFAULT NULL,
                  `Level` int(2) NOT NULL DEFAULT '0',
                  `CurrentFunds` int(4) NOT NULL DEFAULT '0',
                  `IdBilling` varchar(255) NOT NULL DEFAULT '',
                  `Community` char(2) NOT NULL DEFAULT '--',
                  `Newsletter` tinyint(1) NOT NULL DEFAULT '1',
                  `Account` varchar(64) NOT NULL DEFAULT '',
                  `ChoiceSubLength` tinyint(2) NOT NULL DEFAULT '0',
                  `CurrentSubLength` varchar(255) NOT NULL DEFAULT '0',
                  `ValidIdBilling` int(4) NOT NULL DEFAULT '0',
                  `GMId` int(4) NOT NULL DEFAULT '0',
                  `ExtendedPrivilege` varchar(128) NOT NULL DEFAULT '',
                  `ToolsGroup` varchar(20) NOT NULL DEFAULT '',
                  `Unsubscribe` date NOT NULL DEFAULT '0000-00-00',
                  `SubDate` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  `SubIp` varchar(20) NOT NULL DEFAULT '',
                  `SecurePassword` varchar(32) NOT NULL DEFAULT '',
                  `LastInvoiceEmailCheck` date NOT NULL DEFAULT '0000-00-00',
                  `FromSource` varchar(8) NOT NULL DEFAULT '',
                  `ValidMerchantCode` varchar(13) NOT NULL DEFAULT '',
                  `PBC` tinyint(1) NOT NULL DEFAULT '0',
                  `ApiKeySeed` varchar(8) DEFAULT NULL,
                  PRIMARY KEY (`UId`),
                  KEY `LoginIndex` (`Login`),
                  KEY `GroupIndex` (`GroupName`),
                  KEY `ToolsGroup` (`ToolsGroup`),
                  KEY `CurrentSubLength` (`CurrentSubLength`),
                  KEY `Community` (`Community`),
                  KEY `Email` (`Email`),
                  KEY `GMId` (`GMId`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users informations for login system' AUTO_INCREMENT=1 ;     

                GRANT ALL ON `" . $cfg['db']['shard']['name'] ."`.* TO `" . $cfg['db']['shard']['user'] ."`@localhost;
            ";
            $dbs->executeWithoutParams($sql);
            print "The shard database was correctly installed! <br />";
            
            
            //Now create an admin account!
            $hashpass = crypt("admin", Users::generateSALT());
            $params = array(
              'name' => "admin",
              'pass' => $hashpass,
              'mail' => "admin@admin.com",
              'permission' => 3,
              'lang' => "en"
            );
            try{
                $dbw = new DBLayer("web");
                $user_id = $dbw->executeReturnId("INSERT INTO ams_user (Login, Password, Email, Permission, Language) VALUES (:name, :pass, :mail, :permission, :lang)",$params);
                Users::createUser($params, $user_id);
                $dbl = new DBLayer("lib");
                $dbl->execute("UPDATE ticket_user SET Permission = 3 WHERE TUserId = :user_id",array('user_id' => $user_id));
                print "The admin account is created, you can login with id: admin, pass: admin!";
            }catch (PDOException $e){
                print "There was an error while creating the admin account! ";
            }
            
            
            //copy config.default.php to config.php!
            if (!file_exists('../config.php')) {
                if (!copy('../config.default.php', '../config.php')) {
                    echo "failed to copy ../config.php ...\n";
                }
            }
            echo '<br><a href="'.$_SERVER['REQUEST_URI'].'" >Reload!</a> ';
            exit;
            

            
        }catch (PDOException $e) {
            //go to error page or something, because can't access website db
            print "There was an error while installing";
            print_r($e);
        }
    }
        
