<?php

    /**
    * This script will install all databases related to the Ryzom AMS and it will generate an admin account..
    * @author Daan Janssens, mentored by Matthew Lagoe
    */

    //set permissions
    if(is_writable('../../../www/login/logs')) {
        echo "failed to get write permissions on logs";
        exit;
    }
    if(is_writable('../../../admin/graphs_output')) {
        echo "failed to get write permissions on graphs_output";
        exit;
    }
    if(is_writable('../../../admin/templates/default_c')) {
        echo "failed to get write permissions on default_c";
        exit;
    }
    if(is_writable('../../www')) {
        echo "failed to get write permissions on www";
        exit;
    }
    if(is_writable('../../www/html/cache')) {
        echo "failed to get write permissions on cache";
        exit;
    }
    if(is_writable('../../www/html/templates_c')) {
        echo "failed to get write permissions on templates_c";
        exit;
    }
    
    if (!isset($_POST['function'])) { 
        //require the pages that are being needed.
        require_once( '../config.default.php' );
        require_once( $AMS_LIB.'/libinclude.php' );
        ini_set( "display_errors", true );
        error_reporting( E_ALL );

        $return = array();
        $return['no_visible_elements'] = false;
            
        helpers :: loadTemplate( "install" , $return );
        exit;
    } else {
    
        ini_set( "display_errors", true );
        error_reporting( E_ALL );
        
        if (file_exists('../config.php')) {
            require_once( '../config.php' );
        } else {
            //copy config.default.php to config.php!
            if (!file_exists('../config.php')) {
                if (!copy('../config.default.php', '../config.php')) {
                    echo "failed to copy ../config.php ...\n";
                    echo '<br><a href="'.$_SERVER['REQUEST_URI'].'" >Reload!</a> ';
                    exit;
                } else {
                    require_once( '../config.php' );
                }
            }
        }

        //var used to access the DB;
        global $cfg;
        
        try{
            //SETUP THE WWW DB
            $dbw = new DBLayer("install", "web");
            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['web']['name'] ."` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
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
                ) ENGINE = InnoDB;
            GRANT ALL ON `" . $cfg['db']['web']['name'] ."`.* TO `" . $cfg['db']['web']['user'] ."`@".$cfg['db']['web']['host']." identified by '".$cfg['db']['web']['pass']."';
            ";
            $dbw->executeWithoutParams($sql);
            
            //SETUP THE AMS_LIB DB
            $dbl = new DBLayer("install", "lib");

            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
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
            GRANT ALL ON `" . $cfg['db']['lib']['name'] ."`.* TO `" . $cfg['db']['lib']['user'] ."`@".$cfg['db']['lib']['host']." identified by '".$cfg['db']['lib']['pass']."';
            ";
            $dbl->executeWithoutParams($sql);
            print "The Lib & Web database were correctly installed! <br />";
            
            //SETUP THE SHARD DB
            $dbs = new DBLayer("install", "shard");
            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['shard']['name'] ."` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
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

                GRANT ALL ON `" . $cfg['db']['shard']['name'] ."`.* TO `" . $cfg['db']['shard']['user'] ."`@".$cfg['db']['shard']['host']." identified by '".$cfg['db']['shard']['pass']."';
            ";
            $dbs->executeWithoutParams($sql);
            print "The shard database was correctly installed! <br />";
            
            //SETUP THE Nel_Tool DB
            $dbn = new DBLayer("install", "tool");
            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['tool']['name'] ."`;
                USE `". $cfg['db']['tool']['name'] . "`;
                                
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['tool']['name'] ."`  DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
                USE `" . $cfg['db']['tool']['name'] ."`;

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
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=13 ;

                INSERT INTO `neltool_annotations` (`annotation_id`, `annotation_domain_id`, `annotation_shard_id`, `annotation_data`, `annotation_user_name`, `annotation_date`) VALUES
                (12, NULL, 106, 'Welcome to the Shard Admin Website!', 'vl', 1272378352);

                CREATE TABLE IF NOT EXISTS `neltool_applications` (
                  `application_id` int(11) NOT NULL AUTO_INCREMENT,
                  `application_name` varchar(64) NOT NULL DEFAULT '',
                  `application_uri` varchar(255) NOT NULL DEFAULT '',
                  `application_restriction` varchar(64) NOT NULL DEFAULT '',
                  `application_order` int(11) NOT NULL DEFAULT '0',
                  `application_visible` int(11) NOT NULL DEFAULT '0',
                  `application_icon` varchar(128) NOT NULL DEFAULT '',
                  PRIMARY KEY (`application_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=40 ;

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
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=13 ;

                INSERT INTO `neltool_domains` (`domain_id`, `domain_name`, `domain_as_host`, `domain_as_port`, `domain_rrd_path`, `domain_las_admin_path`, `domain_las_local_path`, `domain_application`, `domain_sql_string`, `domain_hd_check`, `domain_mfs_web`, `domain_cs_sql_string`) VALUES
                (12, 'ryzom_open', '127.0.0.1', 46700, '/home/ryzom/code/ryzom/server/save_shard/rrd_graphs', '', '', 'ryzom_open', 'mysql://shard@127.0.0.1/ring_open', 0, '', 'mysql://shard@127.0.0.1/atrium_forums');

                CREATE TABLE IF NOT EXISTS `neltool_groups` (
                  `group_id` int(11) NOT NULL AUTO_INCREMENT,
                  `group_name` varchar(32) NOT NULL DEFAULT 'NewGroup',
                  `group_level` int(11) NOT NULL DEFAULT '0',
                  `group_default` int(11) NOT NULL DEFAULT '0',
                  `group_active` int(11) NOT NULL DEFAULT '0',
                  `group_default_domain_id` tinyint(3) unsigned DEFAULT NULL,
                  `group_default_shard_id` tinyint(3) unsigned DEFAULT NULL,
                  PRIMARY KEY (`group_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=12 ;

                INSERT INTO `neltool_groups` (`group_id`, `group_name`, `group_level`, `group_default`, `group_active`, `group_default_domain_id`, `group_default_shard_id`) VALUES
                (1, 'AdminGroup', 0, 0, 1, 12, 106),
                (2, 'NevraxGroup', 0, 1, 1, NULL, NULL),
                (3, 'AdminDebugGroup', 10, 0, 1, 9, 56),
                (4, 'SupportSGMGroup', 0, 0, 1, NULL, NULL),
                (5, 'NevraxATSGroup', 0, 0, 1, NULL, NULL),
                (6, 'SupportGMGroup', 0, 0, 1, NULL, NULL),
                (7, 'SupportReadOnlyGroup', 0, 0, 1, NULL, NULL),
                (8, 'NevraxLevelDesigners', 0, 0, 1, NULL, NULL),
                (9, 'NevraxReadOnlyGroup', 0, 0, 1, 9, 56),
                (10, 'YubDevGroup', 0, 0, 1, 12, 106),
                (11, 'GuestGroup', 0, 0, 1, 12, 106);

                CREATE TABLE IF NOT EXISTS `neltool_group_applications` (
                  `group_application_id` int(11) NOT NULL AUTO_INCREMENT,
                  `group_application_group_id` int(11) NOT NULL DEFAULT '0',
                  `group_application_application_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`group_application_id`),
                  KEY `group_application_group_id` (`group_application_group_id`),
                  KEY `group_application_application_id` (`group_application_application_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=966 ;

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

                CREATE TABLE IF NOT EXISTS `neltool_group_domains` (
                  `group_domain_id` int(11) NOT NULL AUTO_INCREMENT,
                  `group_domain_group_id` int(11) NOT NULL DEFAULT '0',
                  `group_domain_domain_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`group_domain_id`),
                  KEY `group_domain_group_id` (`group_domain_group_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=90 ;

                INSERT INTO `neltool_group_domains` (`group_domain_id`, `group_domain_group_id`, `group_domain_domain_id`) VALUES
                (79, 1, 9),
                (84, 3, 3),
                (78, 1, 8),
                (43, 2, 1),
                (20, 4, 4),
                (80, 1, 1),
                (77, 1, 3),
                (40, 5, 4),
                (21, 4, 1),
                (22, 6, 1),
                (42, 2, 4),
                (76, 1, 12),
                (83, 3, 12),
                (75, 1, 2),
                (41, 5, 8),
                (44, 2, 8),
                (82, 3, 2),
                (74, 1, 4),
                (73, 9, 9),
                (81, 3, 4),
                (85, 3, 8),
                (86, 3, 9),
                (87, 3, 1),
                (88, 10, 12),
                (89, 11, 12);

                CREATE TABLE IF NOT EXISTS `neltool_group_shards` (
                  `group_shard_id` int(11) NOT NULL AUTO_INCREMENT,
                  `group_shard_group_id` int(11) NOT NULL DEFAULT '0',
                  `group_shard_shard_id` int(11) NOT NULL DEFAULT '0',
                  `group_shard_domain_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`group_shard_id`),
                  KEY `group_shard_group_id` (`group_shard_group_id`),
                  KEY `group_shard_domain_id` (`group_shard_domain_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1522 ;

                INSERT INTO `neltool_group_shards` (`group_shard_id`, `group_shard_group_id`, `group_shard_shard_id`, `group_shard_domain_id`) VALUES
                (1513, 3, 43, 1),
                (1473, 1, 42, 1),
                (1472, 1, 2, 1),
                (1471, 1, 3, 1),
                (1470, 1, 1, 1),
                (1512, 3, 46, 1),
                (1511, 3, 45, 1),
                (1510, 3, 6, 1),
                (1509, 3, 5, 1),
                (1508, 3, 58, 9),
                (1507, 3, 102, 9),
                (1506, 3, 103, 9),
                (841, 2, 37, 8),
                (840, 2, 36, 8),
                (839, 2, 31, 8),
                (838, 2, 47, 8),
                (837, 2, 32, 8),
                (836, 2, 30, 8),
                (1469, 1, 44, 1),
                (1468, 1, 43, 1),
                (1467, 1, 46, 1),
                (1466, 1, 45, 1),
                (1465, 1, 6, 1),
                (1464, 1, 5, 1),
                (1463, 1, 58, 9),
                (1505, 3, 104, 9),
                (1504, 3, 57, 9),
                (1488, 3, 10, 2),
                (1487, 3, 14, 2),
                (1493, 3, 54, 3),
                (1486, 3, 8, 2),
                (1485, 3, 13, 2),
                (1503, 3, 56, 9),
                (1502, 3, 40, 8),
                (1501, 3, 37, 8),
                (1500, 3, 36, 8),
                (1499, 3, 31, 8),
                (1498, 3, 47, 8),
                (1497, 3, 32, 8),
                (1496, 3, 30, 8),
                (1462, 1, 102, 9),
                (1461, 1, 103, 9),
                (1492, 3, 53, 3),
                (1460, 1, 104, 9),
                (1459, 1, 57, 9),
                (1458, 1, 56, 9),
                (1457, 1, 40, 8),
                (903, 5, 37, 8),
                (902, 5, 36, 8),
                (901, 5, 31, 8),
                (900, 5, 47, 8),
                (899, 5, 32, 8),
                (898, 5, 30, 8),
                (897, 5, 39, 8),
                (1456, 1, 37, 8),
                (652, 4, 26, 4),
                (651, 4, 20, 4),
                (650, 4, 19, 4),
                (1491, 3, 15, 3),
                (1455, 1, 36, 8),
                (896, 5, 41, 8),
                (1490, 3, 106, 12),
                (1454, 1, 31, 8),
                (895, 5, 18, 4),
                (894, 5, 26, 4),
                (893, 5, 20, 4),
                (646, 4, 23, 4),
                (645, 4, 22, 4),
                (644, 4, 21, 4),
                (835, 2, 39, 8),
                (834, 2, 41, 8),
                (833, 2, 4, 1),
                (832, 2, 44, 1),
                (831, 2, 43, 1),
                (830, 2, 42, 1),
                (829, 2, 2, 1),
                (828, 2, 46, 1),
                (827, 2, 45, 1),
                (826, 2, 3, 1),
                (825, 2, 1, 1),
                (824, 2, 6, 1),
                (892, 5, 19, 4),
                (1495, 3, 39, 8),
                (1484, 3, 7, 2),
                (891, 5, 24, 4),
                (1489, 3, 107, 12),
                (1483, 3, 18, 4),
                (1482, 3, 26, 4),
                (1481, 3, 20, 4),
                (1480, 3, 19, 4),
                (1479, 3, 24, 4),
                (1453, 1, 47, 8),
                (1452, 1, 32, 8),
                (1474, 1, 4, 1),
                (887, 5, 23, 4),
                (886, 5, 22, 4),
                (1451, 1, 30, 8),
                (1450, 1, 39, 8),
                (1449, 1, 41, 8),
                (1448, 1, 54, 3),
                (1447, 1, 53, 3),
                (885, 5, 21, 4),
                (904, 5, 40, 8),
                (884, 5, 17, 4),
                (823, 2, 5, 1),
                (822, 2, 18, 4),
                (821, 2, 26, 4),
                (820, 2, 20, 4),
                (819, 2, 19, 4),
                (818, 2, 24, 4),
                (1446, 1, 15, 3),
                (1385, 9, 58, 9),
                (1445, 1, 106, 12),
                (1444, 1, 107, 12),
                (1443, 1, 10, 2),
                (1478, 3, 23, 4),
                (1477, 3, 22, 4),
                (1494, 3, 41, 8),
                (814, 2, 23, 4),
                (813, 2, 22, 4),
                (812, 2, 21, 4),
                (653, 4, 42, 1),
                (654, 4, 43, 1),
                (655, 4, 44, 1),
                (1384, 9, 102, 9),
                (842, 2, 40, 8),
                (1383, 9, 103, 9),
                (1382, 9, 104, 9),
                (811, 2, 17, 4),
                (1381, 9, 57, 9),
                (1442, 1, 14, 2),
                (1476, 3, 21, 4),
                (1441, 1, 8, 2),
                (1440, 1, 13, 2),
                (1380, 9, 56, 9),
                (1439, 1, 7, 2),
                (1438, 1, 18, 4),
                (1437, 1, 26, 4),
                (1436, 1, 20, 4),
                (1435, 1, 19, 4),
                (1434, 1, 24, 4),
                (1433, 1, 23, 4),
                (1432, 1, 22, 4),
                (1431, 1, 21, 4),
                (1430, 1, 17, 4),
                (1475, 3, 17, 4),
                (1514, 3, 44, 1),
                (1515, 3, 1, 1),
                (1516, 3, 3, 1),
                (1517, 3, 2, 1),
                (1518, 3, 42, 1),
                (1519, 3, 4, 1),
                (1520, 10, 106, 12),
                (1521, 11, 106, 12);

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
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

                CREATE TABLE IF NOT EXISTS `neltool_logs` (
                  `logs_id` int(11) NOT NULL AUTO_INCREMENT,
                  `logs_user_name` varchar(32) NOT NULL DEFAULT '0',
                  `logs_date` int(11) NOT NULL DEFAULT '0',
                  `logs_data` varchar(255) NOT NULL DEFAULT '',
                  PRIMARY KEY (`logs_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

                CREATE TABLE IF NOT EXISTS `neltool_notes` (
                  `note_id` int(11) NOT NULL AUTO_INCREMENT,
                  `note_user_id` int(11) NOT NULL DEFAULT '0',
                  `note_title` varchar(128) NOT NULL DEFAULT '',
                  `note_data` text NOT NULL,
                  `note_date` int(11) NOT NULL DEFAULT '0',
                  `note_active` int(11) NOT NULL DEFAULT '0',
                  `note_global` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`note_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=11 ;

                INSERT INTO `neltool_notes` (`note_id`, `note_user_id`, `note_title`, `note_data`, `note_date`, `note_active`, `note_global`) VALUES
                (2, 27, 'Welcome', 'Welcome to the shard administration website!\r\n\r\nThis website is used to monitor and restart shards.\r\n\r\nIt also gives some player characters informations.', 1272378065, 1, 1),
                (3, 27, 'Shard Start', '# At the same time : NS and TS\r\n[1 min] : all MS, you can boot them all at the same time\r\n[1 min] : IOS\r\n[3 mins] : GMPS\r\n[3 mins] : EGS\r\n[5 mins] : AI Fyros\r\n[1 min 30] : AI Zorai\r\n[1 min 30] : AI Matis\r\n[1 min 30] : AI TNP\r\n[1 min 30] : AI NPE\r\n[1 min 30] : AI Tryker\r\n[1 min 30] : All FS and SBS at the same time\r\n[30 secs] : WS (atm the WS starts in OPEN mode by default, so be fast before CSR checkage, fix for that inc soon)\r\n\r\nNOTE: you can check the uptime for those timers in the right column of the admin tool: UpTime\r\n', 1158751126, 1, 0),
                (5, 27, 'shutting supplementary', 'the writing wont change when lock the ws\r\n\r\nuntick previous boxes as you shut down\r\n\r\nwait 5 between the ws and the egs ie egs is 5 past rest is 10 past', 1153395380, 1, 0),
                (4, 27, 'Shard Stop', '1. Broadcast to warn players\r\n\r\n2. 10 mins before shutdown, lock the WS\r\n\r\n3. At the right time shut down WS\r\n\r\n4. Shut down EGS\r\nOnly the EGS. Wait 5 reals minutes. Goal is to give enough time to egs, in order to save all the info he has to, and letting him sending those message to all services who need it.\r\n\r\n5. Shut down the rest, et voil&agrave;, you&#039;re done.', 1153314198, 1, 0),
                (6, 27, 'Start (EGS to high?)', 'If [EGS] is to high on startup:\r\n\r\n[shut down egs]\r\n[5 mins]\r\n\r\n[IOS] &amp; [GPMS] (shut down at same time)\r\n\r\nAfter the services are down follow &quot;UP&quot; process with timers again.\r\n\r\nIOS\r\n[3 mins]\r\nGPMS\r\n[3 mins]\r\nEGS\r\n[5 mins]\r\nbla bla...', 1153395097, 1, 0),
                (7, 27, 'opening if the egs is too high on reboot', '&lt;kadael&gt; here my note on admin about egs to high on startup\r\n&lt;kadael&gt; ---\r\n&lt;kadael&gt; If [EGS] is to high on startup:\r\n&lt;kadael&gt; [shut down egs]\r\n&lt;kadael&gt; [5 mins]\r\n&lt;kadael&gt; [IOS] &amp; [GPMS] (at same time shut down )\r\n&lt;kadael&gt; after the services are down follow &quot;UP&quot; process with timers again.\r\n&lt;kadael&gt; IOS\r\n&lt;kadael&gt; [3 mins]\r\n&lt;kadael&gt; GPMS\r\n&lt;kadael&gt; [3 mins]\r\n&lt;kadael&gt; EGS\r\n&lt;kadael&gt; [5 mins]\r\n&lt;kadael&gt; bla bla...\r\n&lt;kadael&gt; ---', 1153395362, 1, 0),
                (10, 27, 'Ring points', 'Commande pour donner tout les points ring &agrave; tout le monde :\r\n\r\nDans le DSS d&#039;un Shard Ring entrer : DefaultCharRingAccess f7:j7:l6:d7:p13:g9:a9', 1155722296, 1, 0),
                (9, 27, 'Start (EGS to high?)', 'If [EGS] is to high on startup: \r\n  \r\n [shut down egs] \r\n [5 mins] \r\n  \r\n [IOS] &amp; [GPMS] (shut down at same time) \r\n  \r\n After the services are down follow &quot;UP&quot; process with timers again. \r\n  \r\n IOS \r\n [3 mins] \r\n GPMS \r\n [3 mins] \r\n EGS \r\n [5 mins] \r\n bla bla...', 1153929658, 1, 0);

                CREATE TABLE IF NOT EXISTS `neltool_restart_groups` (
                  `restart_group_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `restart_group_name` varchar(50) DEFAULT NULL,
                  `restart_group_list` varchar(50) DEFAULT NULL,
                  `restart_group_order` varchar(50) DEFAULT NULL,
                  PRIMARY KEY (`restart_group_id`),
                  UNIQUE KEY `restart_group_id` (`restart_group_id`),
                  KEY `restart_group_id_2` (`restart_group_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=6 ;

                INSERT INTO `neltool_restart_groups` (`restart_group_id`, `restart_group_name`, `restart_group_list`, `restart_group_order`) VALUES
                (1, 'Low Level', 'rns,ts,ms', '1'),
                (3, 'Mid Level', 'ios,gpms,egs', '2'),
                (4, 'High Level', 'ais', '3'),
                (5, 'Front Level', 'fes,sbs,dss,rws', '4');

                CREATE TABLE IF NOT EXISTS `neltool_restart_messages` (
                  `restart_message_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `restart_message_name` varchar(20) DEFAULT NULL,
                  `restart_message_value` varchar(128) DEFAULT NULL,
                  `restart_message_lang` varchar(5) DEFAULT NULL,
                  PRIMARY KEY (`restart_message_id`),
                  UNIQUE KEY `restart_message_id` (`restart_message_id`),
                  KEY `restart_message_id_2` (`restart_message_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=11 ;

                INSERT INTO `neltool_restart_messages` (`restart_message_id`, `restart_message_name`, `restart_message_value`, `restart_message_lang`) VALUES
                (5, 'reboot', 'The shard is about to go down. Please find a safe location and log out.', 'en'),
                (4, 'reboot', 'Le serveur va redemarrer dans minutes. Merci de vous deconnecter en lieu sur.', 'fr'),
                (6, 'reboot', 'Der Server wird heruntergefahren. Findet eine sichere Stelle und logt aus.', 'de'),
                (10, 'reboot', 'Arret du serveur dans minutes', 'fr');

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
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

                CREATE TABLE IF NOT EXISTS `neltool_shards` (
                  `shard_id` int(11) NOT NULL AUTO_INCREMENT,
                  `shard_name` varchar(128) NOT NULL DEFAULT '',
                  `shard_as_id` varchar(255) NOT NULL DEFAULT '0',
                  `shard_domain_id` int(11) NOT NULL DEFAULT '0',
                  `shard_lang` char(2) NOT NULL DEFAULT 'en',
                  `shard_restart` int(10) unsigned NOT NULL DEFAULT '0',
                  PRIMARY KEY (`shard_id`),
                  KEY `shard_domain_id` (`shard_domain_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=107 ;

                INSERT INTO `neltool_shards` (`shard_id`, `shard_name`, `shard_as_id`, `shard_domain_id`, `shard_lang`, `shard_restart`) VALUES
                (106, 'Open', 'open', 12, 'en', 0);

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
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

                CREATE TABLE IF NOT EXISTS `neltool_stats_hd_times` (
                  `hd_domain_id` int(11) NOT NULL DEFAULT '0',
                  `hd_last_time` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`hd_domain_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1;

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
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=33 ;

                INSERT INTO `neltool_users` (`user_id`, `user_name`, `user_password`, `user_group_id`, `user_created`, `user_active`, `user_logged_last`, `user_logged_count`, `user_menu_style`) VALUES
                (27, 'admin', '084e0343a0486ff05530df6c705c8bb4', 1, 1213886454, 1, 1273158945, 382, 2),
                (32, 'guest', '084e0343a0486ff05530df6c705c8bb4', 1, 1272379014, 1, 1277452380, 274, 2);

                CREATE TABLE IF NOT EXISTS `neltool_user_applications` (
                  `user_application_id` int(11) NOT NULL AUTO_INCREMENT,
                  `user_application_user_id` int(11) NOT NULL DEFAULT '0',
                  `user_application_application_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`user_application_id`),
                  KEY `user_application_user_id` (`user_application_user_id`),
                  KEY `user_application_application_id` (`user_application_application_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=22 ;

                INSERT INTO `neltool_user_applications` (`user_application_id`, `user_application_user_id`, `user_application_application_id`) VALUES
                (8, 12, 33),
                (20, 6, 31),
                (19, 6, 34),
                (9, 12, 31),
                (21, 10, 34);

                CREATE TABLE IF NOT EXISTS `neltool_user_domains` (
                  `user_domain_id` int(11) NOT NULL AUTO_INCREMENT,
                  `user_domain_user_id` int(11) NOT NULL DEFAULT '0',
                  `user_domain_domain_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`user_domain_id`),
                  KEY `user_domain_user_id` (`user_domain_user_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=20 ;

                INSERT INTO `neltool_user_domains` (`user_domain_id`, `user_domain_user_id`, `user_domain_domain_id`) VALUES
                (5, 6, 2),
                (9, 22, 1),
                (10, 23, 4),
                (4, 12, 3),
                (6, 6, 3),
                (11, 23, 2),
                (12, 23, 1),
                (13, 23, 8),
                (18, 15, 1),
                (17, 15, 2),
                (19, 31, 9);

                CREATE TABLE IF NOT EXISTS `neltool_user_shards` (
                  `user_shard_id` int(11) NOT NULL AUTO_INCREMENT,
                  `user_shard_user_id` int(11) NOT NULL DEFAULT '0',
                  `user_shard_shard_id` int(11) NOT NULL DEFAULT '0',
                  `user_shard_domain_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`user_shard_id`),
                  KEY `user_shard_user_id` (`user_shard_user_id`),
                  KEY `user_shard_domain_id` (`user_shard_domain_id`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=166 ;

                INSERT INTO `neltool_user_shards` (`user_shard_id`, `user_shard_user_id`, `user_shard_shard_id`, `user_shard_domain_id`) VALUES
                (1, 8, 1, 1),
                (2, 9, 2, 1),
                (68, 7, 3, 1),
                (143, 6, 4, 1),
                (142, 6, 2, 1),
                (141, 6, 45, 1),
                (140, 6, 3, 1),
                (90, 23, 26, 4),
                (89, 23, 20, 4),
                (13, 14, 1, 1),
                (14, 14, 3, 1),
                (15, 14, 2, 1),
                (139, 6, 1, 1),
                (74, 17, 2, 1),
                (73, 17, 45, 1),
                (72, 17, 3, 1),
                (71, 17, 1, 1),
                (70, 17, 18, 4),
                (88, 23, 19, 4),
                (87, 23, 24, 4),
                (83, 23, 23, 4),
                (82, 23, 22, 4),
                (81, 23, 21, 4),
                (34, 12, 15, 3),
                (36, 18, 2, 1),
                (138, 6, 7, 2),
                (80, 23, 17, 4),
                (79, 22, 45, 1),
                (78, 22, 3, 1),
                (77, 21, 45, 1),
                (76, 21, 3, 1),
                (75, 17, 4, 1),
                (69, 7, 45, 1),
                (146, 6, 54, 3),
                (91, 23, 18, 4),
                (92, 23, 7, 2),
                (93, 23, 13, 2),
                (94, 23, 8, 2),
                (95, 23, 14, 2),
                (145, 6, 53, 3),
                (97, 23, 10, 2),
                (144, 6, 15, 3),
                (99, 23, 5, 1),
                (100, 23, 6, 1),
                (101, 23, 1, 1),
                (102, 23, 3, 1),
                (103, 23, 45, 1),
                (104, 23, 46, 1),
                (105, 23, 2, 1),
                (106, 23, 42, 1),
                (107, 23, 43, 1),
                (108, 23, 44, 1),
                (109, 23, 4, 1),
                (110, 23, 41, 8),
                (111, 23, 39, 8),
                (112, 23, 30, 8),
                (113, 23, 32, 8),
                (114, 23, 47, 8),
                (115, 23, 31, 8),
                (116, 23, 36, 8),
                (117, 23, 37, 8),
                (118, 23, 40, 8),
                (156, 15, 45, 1),
                (155, 15, 3, 1),
                (154, 15, 1, 1),
                (153, 15, 6, 1),
                (152, 15, 5, 1),
                (151, 15, 10, 2),
                (150, 15, 14, 2),
                (149, 15, 8, 2),
                (148, 15, 13, 2),
                (147, 15, 7, 2),
                (157, 15, 46, 1),
                (158, 15, 2, 1),
                (159, 15, 42, 1),
                (160, 15, 43, 1),
                (161, 15, 44, 1),
                (162, 15, 4, 1),
                (163, 31, 57, 9),
                (164, 31, 104, 9),
                (165, 31, 103, 9);   

                GRANT ALL ON `" . $cfg['db']['tool']['name'] ."`.* TO `" . $cfg['db']['tool']['user'] ."`@".$cfg['db']['tool']['host']." identified by '".$cfg['db']['tool']['pass']."';
            ";
            $dbn->executeWithoutParams($sql);
            print "The nel_tool database was correctly installed! <br />";
            
                        //SETUP THE OPEN_SHARD DB
            $dbw = new DBLayer("install", "ring");
            $sql = "
                CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['ring']['name'] ."` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
                USE `" . $cfg['db']['ring']['name'] ."`;

                CREATE TABLE IF NOT EXISTS `characters` (
                  `char_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `char_name` varchar(20) NOT NULL DEFAULT '',
                  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `guild_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `best_combat_level` int(10) unsigned NOT NULL DEFAULT '0',
                  `home_mainland_session_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `ring_access` varchar(63) NOT NULL DEFAULT '',
                  `race` enum('r_fyros','r_matis','r_tryker','r_zorai') NOT NULL DEFAULT 'r_fyros',
                  `civilisation` enum('c_neutral','c_fyros','c_fyros','c_matis','c_tryker','c_zorai') NOT NULL DEFAULT 'c_neutral',
                  `cult` enum('c_neutral','c_kami','c_karavan') NOT NULL DEFAULT 'c_neutral',
                  `current_session` int(11) unsigned NOT NULL DEFAULT '0',
                  `rrp_am` int(11) unsigned NOT NULL DEFAULT '0',
                  `rrp_masterless` int(11) unsigned NOT NULL DEFAULT '0',
                  `rrp_author` int(11) unsigned NOT NULL DEFAULT '0',
                  `newcomer` tinyint(1) NOT NULL DEFAULT '1',
                  `creation_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  `last_played_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  PRIMARY KEY (`char_id`),
                  UNIQUE KEY `char_name_idx` (`char_name`,`home_mainland_session_id`),
                  KEY `user_id_idx` (`user_id`),
                  KEY `guild_idx` (`guild_id`),
                  KEY `guild_id_idx` (`guild_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1;

                CREATE TABLE IF NOT EXISTS `folder` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `owner` int(10) unsigned NOT NULL DEFAULT '0',
                  `title` varchar(40) NOT NULL DEFAULT '',
                  `comments` text NOT NULL,
                  PRIMARY KEY (`Id`),
                  KEY `owner_idx` (`owner`),
                  KEY `title_idx` (`title`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `folder_access` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `folder_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
                  PRIMARY KEY (`Id`),
                  KEY `folder_id_idx` (`folder_id`),
                  KEY `user_idx` (`user_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `guild_invites` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `guild_id` int(10) unsigned NOT NULL DEFAULT '0',
                  PRIMARY KEY (`Id`),
                  KEY `guild_id_idx` (`guild_id`),
                  KEY `session_id_idx` (`session_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `guilds` (
                  `guild_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `guild_name` varchar(50) NOT NULL DEFAULT '',
                  `shard_id` int(11) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`guild_id`),
                  KEY `shard_id_idx` (`shard_id`),
                  KEY `guild_name_idx` (`guild_name`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;


                CREATE TABLE IF NOT EXISTS `journal_entry` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `author` int(10) unsigned NOT NULL DEFAULT '0',
                  `type` enum('jet_credits','jet_notes') NOT NULL DEFAULT 'jet_notes',
                  `text` text NOT NULL,
                  `time_stamp` datetime NOT NULL DEFAULT '2005-09-07 12:41:33',
                  PRIMARY KEY (`Id`),
                  KEY `session_id_idx` (`session_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `known_users` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `owner` int(10) unsigned NOT NULL DEFAULT '0',
                  `targer_user` int(10) unsigned NOT NULL DEFAULT '0',
                  `targer_character` int(10) unsigned NOT NULL DEFAULT '0',
                  `relation_type` enum('rt_friend','rt_banned','rt_friend_dm') NOT NULL DEFAULT 'rt_friend',
                  `comments` varchar(255) NOT NULL DEFAULT '',
                  PRIMARY KEY (`Id`),
                  KEY `user_index` (`owner`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `mfs_erased_mail_series` (
                  `erased_char_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `erased_char_name` varchar(32) NOT NULL DEFAULT '',
                  `erased_series` int(11) unsigned NOT NULL AUTO_INCREMENT,
                  `erase_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  PRIMARY KEY (`erased_series`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `mfs_guild_thread` (
                  `thread_id` int(11) NOT NULL AUTO_INCREMENT,
                  `guild_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `topic` varchar(255) NOT NULL DEFAULT '',
                  `author_name` varchar(32) NOT NULL DEFAULT '',
                  `last_post_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  `post_count` int(11) unsigned NOT NULL DEFAULT '0',
                  PRIMARY KEY (`thread_id`),
                  KEY `guild_index` (`guild_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `mfs_guild_thread_message` (
                  `id` int(11) NOT NULL AUTO_INCREMENT,
                  `thread_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `author_name` varchar(32) NOT NULL DEFAULT '',
                  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  `content` text NOT NULL,
                  PRIMARY KEY (`id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `mfs_mail` (
                  `id` int(11) NOT NULL AUTO_INCREMENT,
                  `sender_name` varchar(32) NOT NULL DEFAULT '',
                  `subject` varchar(250) NOT NULL DEFAULT '',
                  `date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  `status` enum('ms_new','ms_read','ms_erased') NOT NULL DEFAULT 'ms_new',
                  `dest_char_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `erase_series` int(11) unsigned NOT NULL DEFAULT '0',
                  `content` text NOT NULL,
                  PRIMARY KEY (`id`),
                  KEY `dest_index` (`dest_char_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `outlands` (
                  `session_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
                  `island_name` text NOT NULL,
                  `billing_instance_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `anim_session_id` int(11) unsigned NOT NULL DEFAULT '0',
                  PRIMARY KEY (`session_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `player_rating` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `scenario_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `rate_fun` tinyint(3) unsigned NOT NULL DEFAULT '0',
                  `rate_difficulty` tinyint(3) unsigned NOT NULL DEFAULT '0',
                  `rate_accessibility` tinyint(3) unsigned NOT NULL DEFAULT '0',
                  `rate_originality` tinyint(3) unsigned NOT NULL DEFAULT '0',
                  `rate_direction` tinyint(3) unsigned NOT NULL DEFAULT '0',
                  `author` int(10) unsigned NOT NULL DEFAULT '0',
                  `rating` int(10) NOT NULL DEFAULT '0',
                  `comments` text NOT NULL,
                  `time_stamp` datetime NOT NULL DEFAULT '2005-09-07 12:41:33',
                  PRIMARY KEY (`Id`),
                  KEY `session_id_idx` (`scenario_id`),
                  KEY `author_idx` (`author`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `ring_users` (
                  `user_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `user_name` varchar(20) NOT NULL DEFAULT '',
                  `user_type` enum('ut_character','ut_pioneer') NOT NULL DEFAULT 'ut_character',
                  `current_session` int(10) unsigned NOT NULL DEFAULT '0',
                  `current_activity` enum('ca_none','ca_play','ca_edit','ca_anim') NOT NULL DEFAULT 'ca_none',
                  `current_status` enum('cs_offline','cs_logged','cs_online') NOT NULL DEFAULT 'cs_offline',
                  `public_level` enum('pl_none','pl_public') NOT NULL DEFAULT 'pl_none',
                  `account_type` enum('at_normal','at_gold') NOT NULL DEFAULT 'at_normal',
                  `content_access_level` varchar(20) NOT NULL DEFAULT '',
                  `description` text NOT NULL,
                  `lang` enum('lang_en','lang_fr','lang_de') NOT NULL DEFAULT 'lang_en',
                  `cookie` varchar(30) NOT NULL DEFAULT '',
                  `current_domain_id` int(10) NOT NULL DEFAULT '-1',
                  `pioneer_char_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `current_char` int(11) NOT NULL DEFAULT '0',
                  `add_privileges` varchar(64) NOT NULL,
                  PRIMARY KEY (`user_id`),
                  UNIQUE KEY `user_name_idx` (`user_name`),
                  KEY `cookie_idx` (`cookie`),
                  KEY `current_session_idx` (`current_session`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;

                CREATE TABLE IF NOT EXISTS `scenario` (
                  `id` int(11) NOT NULL AUTO_INCREMENT,
                  `md5` varchar(64) NOT NULL DEFAULT '',
                  `title` varchar(32) NOT NULL DEFAULT '',
                  `description` text NOT NULL,
                  `author` varchar(32) NOT NULL DEFAULT '',
                  `rrp_total` int(11) unsigned NOT NULL DEFAULT '0',
                  `anim_mode` enum('am_dm','am_autonomous') NOT NULL DEFAULT 'am_dm',
                  `language` varchar(11) NOT NULL DEFAULT '',
                  `orientation` enum('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') NOT NULL DEFAULT 'so_other',
                  `level` enum('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') NOT NULL DEFAULT 'sl_a',
                  `allow_free_trial` tinyint(1) NOT NULL DEFAULT '0',
                  PRIMARY KEY (`id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `scenario_desc` (
                  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `parent_scenario` int(10) unsigned NOT NULL DEFAULT '0',
                  `description` text NOT NULL,
                  `relation_to_parent` enum('rtp_same','rtp_variant','rtp_different') NOT NULL DEFAULT 'rtp_same',
                  `title` varchar(40) NOT NULL DEFAULT '',
                  `num_player` int(10) unsigned NOT NULL DEFAULT '0',
                  `content_access_level` varchar(20) NOT NULL DEFAULT '',
                  PRIMARY KEY (`session_id`),
                  UNIQUE KEY `title_idx` (`title`),
                  KEY `parent_idx` (`parent_scenario`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;


                CREATE TABLE IF NOT EXISTS `session_log` (
                  `id` int(11) NOT NULL DEFAULT '0',
                  `scenario_id` int(11) unsigned NOT NULL DEFAULT '0',
                  `rrp_scored` int(11) unsigned NOT NULL DEFAULT '0',
                  `scenario_point_scored` int(11) unsigned NOT NULL DEFAULT '0',
                  `time_taken` int(11) unsigned NOT NULL DEFAULT '0',
                  `participants` text NOT NULL,
                  `launch_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
                  `owner` varchar(32) NOT NULL DEFAULT '0',
                  `guild_name` varchar(50) DEFAULT NULL,
                  PRIMARY KEY (`id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC;


                CREATE TABLE IF NOT EXISTS `session_participant` (
                  `Id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `session_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `char_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `status` enum('sps_play_subscribed','sps_play_invited','sps_edit_invited','sps_anim_invited','sps_playing','sps_editing','sps_animating') NOT NULL DEFAULT 'sps_play_subscribed',
                  `kicked` tinyint(1) unsigned NOT NULL DEFAULT '0',
                  `session_rated` tinyint(1) unsigned NOT NULL DEFAULT '0',
                  PRIMARY KEY (`Id`),
                  KEY `session_idx` (`session_id`),
                  KEY `user_idx` (`char_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED AUTO_INCREMENT=1 ;


                CREATE TABLE IF NOT EXISTS `sessions` (
                  `session_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
                  `session_type` enum('st_edit','st_anim','st_outland','st_mainland') NOT NULL DEFAULT 'st_edit',
                  `title` varchar(40) NOT NULL DEFAULT '',
                  `owner` int(10) unsigned NOT NULL DEFAULT '0',
                  `plan_date` datetime NOT NULL DEFAULT '2005-09-21 12:41:33',
                  `start_date` datetime NOT NULL DEFAULT '2005-08-31 00:00:00',
                  `description` text NOT NULL,
                  `orientation` enum('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') NOT NULL DEFAULT 'so_other',
                  `level` enum('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') NOT NULL DEFAULT 'sl_a',
                  `rule_type` enum('rt_strict','rt_liberal') NOT NULL DEFAULT 'rt_strict',
                  `access_type` enum('at_public','at_private') NOT NULL DEFAULT 'at_private',
                  `state` enum('ss_planned','ss_open','ss_locked','ss_closed') NOT NULL DEFAULT 'ss_planned',
                  `host_shard_id` int(11) NOT NULL DEFAULT '0',
                  `subscription_slots` int(11) unsigned NOT NULL DEFAULT '0',
                  `reserved_slots` int(10) unsigned NOT NULL DEFAULT '0',
                  `free_slots` int(10) unsigned NOT NULL DEFAULT '0',
                  `estimated_duration` enum('et_short','et_medium','et_long') NOT NULL DEFAULT 'et_short',
                  `final_duration` int(10) unsigned NOT NULL DEFAULT '0',
                  `folder_id` int(10) unsigned NOT NULL DEFAULT '0',
                  `lang` varchar(20) NOT NULL DEFAULT '',
                  `icone` varchar(70) NOT NULL DEFAULT '',
                  `anim_mode` enum('am_dm','am_autonomous') NOT NULL DEFAULT 'am_dm',
                  `race_filter` set('rf_fyros','rf_matis','rf_tryker','rf_zorai') NOT NULL DEFAULT '',
                  `religion_filter` set('rf_kami','rf_karavan','rf_neutral') NOT NULL DEFAULT '',
                  `guild_filter` enum('gf_only_my_guild','gf_any_player') DEFAULT 'gf_only_my_guild',
                  `shard_filter` set('sf_shard00','sf_shard01','sf_shard02','sf_shard03','sf_shard04','sf_shard05','sf_shard06','sf_shard07','sf_shard08','sf_shard09','sf_shard10','sf_shard11','sf_shard12','sf_shard13','sf_shard14','sf_shard15','sf_shard16','sf_shard17','sf_shard18','sf_shard19','sf_shard20','sf_shard21','sf_shard22','sf_shard23','sf_shard24','sf_shard25','sf_shard26','sf_shard27','sf_shard28','sf_shard29','sf_shard30','sf_shard31') NOT NULL DEFAULT 'sf_shard00,sf_shard01,sf_shard02,sf_shard03,sf_shard04,sf_shard05,sf_shard06,sf_shard07,sf_shard08,sf_shard09,sf_shard10,sf_shard11,sf_shard12,sf_shard13,sf_shard14,sf_shard15,sf_shard16,sf_shard17,sf_shard18,sf_shard19,sf_shard20,sf_shard21,sf_shard22,sf_shard23,sf_shard24,sf_shard25,sf_shard26,sf_shard27,sf_shard28,sf_shard29,sf_shard30,sf_shard31',
                  `level_filter` set('lf_a','lf_b','lf_c','lf_d','lf_e','lf_f') NOT NULL DEFAULT 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f',
                  `subscription_closed` tinyint(1) NOT NULL DEFAULT '0',
                  `newcomer` tinyint(1) unsigned zerofill NOT NULL DEFAULT '0',
                  PRIMARY KEY (`session_id`),
                  KEY `owner_idx` (`owner`),
                  KEY `folder_idx` (`folder_id`),
                  KEY `state_type_idx` (`state`,`session_type`)
                ) ENGINE=MyISAM  DEFAULT CHARSET=latin1 ROW_FORMAT=DYNAMIC AUTO_INCREMENT=303 ;

                INSERT INTO `sessions` (`session_id`, `session_type`, `title`, `owner`, `plan_date`, `start_date`, `description`, `orientation`, `level`, `rule_type`, `access_type`, `state`, `host_shard_id`, `subscription_slots`, `reserved_slots`, `free_slots`, `estimated_duration`, `final_duration`, `folder_id`, `lang`, `icone`, `anim_mode`, `race_filter`, `religion_filter`, `guild_filter`, `shard_filter`, `level_filter`, `subscription_closed`, `newcomer`) VALUES
                (302, 'st_mainland', 'open shard mainland', 0, '2005-09-21 12:41:33', '2005-08-31 00:00:00', '', 'so_other', 'sl_a', 'rt_strict', 'at_public', 'ss_planned', 0, 0, 0, 0, 'et_short', 0, 0, 'lang_en', '', 'am_dm', 'rf_fyros,rf_matis,rf_tryker,rf_zorai', 'rf_kami,rf_karavan,rf_neutral', 'gf_any_player', '', 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f', 0, 0);

                CREATE TABLE IF NOT EXISTS `shard` (
                  `shard_id` int(10) NOT NULL DEFAULT '0',
                  `WSOnline` tinyint(1) NOT NULL DEFAULT '0',
                  `MOTD` text NOT NULL,
                  `OldState` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_restricted',
                  `RequiredState` enum('ds_close','ds_dev','ds_restricted','ds_open') NOT NULL DEFAULT 'ds_dev',
                  PRIMARY KEY (`shard_id`)
                ) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED;

                INSERT INTO `shard` (`shard_id`, `WSOnline`, `MOTD`, `OldState`, `RequiredState`) VALUES
                (302, 1, 'Shard up', 'ds_restricted', 'ds_open');
                
                INSERT INTO `sessions` (`session_id`, `session_type`, `title`, `owner`, `plan_date`, `start_date`, `description`, `orientation`, `level`, `rule_type`, `access_type`, `state`, `host_shard_id`, `subscription_slots`, `reserved_slots`, `free_slots`, `estimated_duration`, `final_duration`, `folder_id`, `lang`, `icone`, `anim_mode`, `race_filter`, `religion_filter`, `guild_filter`, `shard_filter`, `level_filter`, `subscription_closed`, `newcomer`) VALUES (302, 'st_mainland', 'open shard mainland', 0, '2005-09-21 12:41:33', '2005-08-31 00:00:00', '', 'so_other', 'sl_a', 'rt_strict', 'at_public', 'ss_planned', 0, 0, 0, 0, 'et_short', 0, 0, 'lang_en', '', 'am_dm', 'rf_fyros,rf_matis,rf_tryker,rf_zorai', 'rf_kami,rf_karavan,rf_neutral', 'gf_any_player', '', 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f', 0, 0);

            GRANT ALL ON `" . $cfg['db']['ring']['name'] ."`.* TO `" . $cfg['db']['ring']['user'] ."`@".$cfg['db']['ring']['host']." identified by '".$cfg['db']['ring']['pass']."';
            ";
            $dbw->executeWithoutParams($sql);
            
            print "The ring_open database was correctly installed! <br />";
            
            
            //Now create an admin account!
            $hashpass = crypt("admin", Users::generateSALT());
            $params = array(
              'name' => "admin",
              'pass' => $hashpass,
              'mail' => "admin@admin.com",
            );
            try{
                $user_id = WebUsers::createWebuser($params['name'], $params['pass'],$params['mail']);
                $result = Webusers::createUser($params, $user_id);
                Users::createPermissions(array($params['name']));
                $dbl = new DBLayer("lib");
                $dbl->execute("UPDATE ticket_user SET Permission = 3 WHERE TUserId = :user_id",array('user_id' => $user_id));
                print "The admin account is created, you can login with id: admin, pass: admin!";
            }catch (PDOException $e){
                print "There was an error while creating the admin account! ";
                print_r($e);
            }


            echo '<br><a href="'.$_SERVER['REQUEST_URI'].'" >Reload!</a> ';
            
            touch("../is_installed");
            
            exit;
            

            
        }catch (PDOException $e) {
            //go to error page or something, because can't access website db
            print "There was an error while installing";
            print_r($e);
        }
    }
        
