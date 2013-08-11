<?php
    require( '../../config.php' );
    require( '../../../ams_lib/libinclude.php' );
    ini_set( "display_errors", true );
    error_reporting( E_ALL );
    
    global $cfg;
    

    try{
        //SETUP THE WWW DB
        $dbw = new DBLayer("web");
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
            PRIMARY KEY (`UId`)
            ) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='contains all users information for ryzom_ams';
            
            );           
        ";
        $dbw->executeWithoutParams($sql);
        
        //SETUP THE AMS_LIB DB
        $dbl = new DBLayer("lib");
        $sql = "
            CREATE DATABASE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`;
            USE `" . $cfg['db']['lib']['name'] ."`;
            DROP TABLE IF EXISTS ams_querycache;
            
            CREATE TABLE ams_querycache (
            `SID` INT NOT NULL AUTO_INCREMENT PRIMARY KEY ,
            `type` VARCHAR( 64 ) NOT NULL ,
            `query` VARCHAR( 512 ) NOT NULL,
            `db` VARCHAR( 80 ) NOT NULL
            );
            

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
            
        INSERT IGNORE INTO `" . $cfg['db']['lib']['name'] ."`.`ticket_category` (`Name`) VALUES ('Hacking'),('Ingame-Bug'),('Website-Bug'),('Installation');
        
                        
        
        -- -----------------------------------------------------
        -- Table `" . $cfg['db']['lib']['name'] ."`.`support_group`
        -- -----------------------------------------------------
        DROP TABLE IF EXISTS `" . $cfg['db']['lib']['name'] ."`.`support_group` ;
        
        CREATE  TABLE IF NOT EXISTS `" . $cfg['db']['lib']['name'] ."`.`support_group` (
          `SGroupId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
          `Name` VARCHAR(22) NOT NULL ,
          `Tag` VARCHAR(7) NOT NULL ,
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
          `Sender` INT(10) UNSIGNED NOT NULL ,
          `UserId` INT(10) UNSIGNED NOT NULL ,
          `MessageId` VARCHAR(45) NOT NULL ,
          PRIMARY KEY (`MailId`) ,
          INDEX `fk_email_ticket_user1` (`Sender` ASC) ,
          INDEX `fk_email_ticket_user2` (`UserId` ASC) ,
          CONSTRAINT `fk_email_ticket_user1`
            FOREIGN KEY (`Sender` )
            REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
            ON DELETE NO ACTION
            ON UPDATE NO ACTION,
          CONSTRAINT `fk_email_ticket_user2`
            FOREIGN KEY (`UserId` )
            REFERENCES `" . $cfg['db']['lib']['name'] ."`.`ticket_user` (`TUserId` )
            ON DELETE NO ACTION
            ON UPDATE NO ACTION)
        ENGINE = InnoDB;

                

        ";
        $dbl->executeWithoutParams($sql);
        print "The Lib & Web database were correctly installed! <br />";
        
        //Now create an admin account!
        $hashpass = crypt("admin", Users::generateSALT());
        $params = array(
          'name' => "admin",
          'pass' => $hashpass,
          'mail' => "admin@admin.com",
          'permission' => 3
        );
        try{
            $dbw = new DBLayer("web");
            $user_id = $dbw->executeReturnId("INSERT INTO ams_user (Login, Password, Email, Permission) VALUES (:name, :pass, :mail, :permission)",$params);
            Users::createUser($params, $user_id);
            $dbl = new DBLayer("lib");
            $dbl->execute("UPDATE ticket_user SET Permission = 3 WHERE TUserId = :user_id",array('user_id' => $user_id));
            print "The admin account is created, you can login with id: admin, pass: admin!";
        }catch (PDOException $e){
            print "There was an error while creating the admin account! ";
        }
        
        
    
        

        
    }catch (PDOException $e) {
        //go to error page or something, because can't access website db
        print "There was an error while installing";
        print_r($e);
    }
    
        
