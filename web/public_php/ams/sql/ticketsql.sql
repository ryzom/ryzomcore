SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';

CREATE SCHEMA IF NOT EXISTS `mydb` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci ;
USE `mydb` ;

-- -----------------------------------------------------
-- Table `mydb`.`ticket_category`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_category` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_category` (
  `TCategoryId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`TCategoryId`) ,
  UNIQUE INDEX `Name_UNIQUE` (`Name` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_user` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_user` (
  `TUserId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Permission` INT(3) NOT NULL DEFAULT 1 ,
  `ExternId` INT(10) UNSIGNED NOT NULL ,
  PRIMARY KEY (`TUserId`) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket` (
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
    REFERENCES `mydb`.`ticket_category` (`TCategoryId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_ticket_ams_user`
    FOREIGN KEY (`Author` )
    REFERENCES `mydb`.`ticket_user` (`TUserId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`assigned`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`assigned` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`assigned` (
  `Ticket` INT UNSIGNED NOT NULL ,
  `User` INT UNSIGNED NOT NULL ,
  INDEX `fk_assigned_ticket_idx` (`Ticket` ASC) ,
  PRIMARY KEY (`Ticket`, `User`) ,
  INDEX `fk_assigned_ams_user_idx` (`User` ASC) ,
  CONSTRAINT `fk_assigned_ticket`
    FOREIGN KEY (`Ticket` )
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_assigned_ams_user`
    FOREIGN KEY (`User` )
    REFERENCES `mydb`.`ticket_user` (`TUserId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`tag`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`tag` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`tag` (
  `TagId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Value` VARCHAR(60) NOT NULL ,
  PRIMARY KEY (`TagId`) ,
  UNIQUE INDEX `Value_UNIQUE` (`Value` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`tagged`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`tagged` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`tagged` (
  `Ticket` INT UNSIGNED NOT NULL ,
  `Tag` INT UNSIGNED NOT NULL ,
  PRIMARY KEY (`Ticket`, `Tag`) ,
  INDEX `fk_tagged_tag_idx` (`Tag` ASC) ,
  CONSTRAINT `fk_tagged_ticket`
    FOREIGN KEY (`Ticket` )
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_tagged_tag`
    FOREIGN KEY (`Tag` )
    REFERENCES `mydb`.`tag` (`TagId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_content`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_content` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_content` (
  `TContentId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Content` TEXT NULL ,
  PRIMARY KEY (`TContentId`) )
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_reply`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_reply` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_reply` (
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
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_ticket_reply_ams_user`
    FOREIGN KEY (`Author` )
    REFERENCES `mydb`.`ticket_user` (`TUserId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_ticket_reply_ticket_content`
    FOREIGN KEY (`Content` )
    REFERENCES `mydb`.`ticket_content` (`TContentId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_group`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_group` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_group` (
  `TGroupId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Title` VARCHAR(80) NOT NULL ,
  PRIMARY KEY (`TGroupId`) ,
  UNIQUE INDEX `Title_UNIQUE` (`Title` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`in_group`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`in_group` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`in_group` (
  `Ticket_Group` INT UNSIGNED NOT NULL ,
  `Ticket` INT UNSIGNED NOT NULL ,
  PRIMARY KEY (`Ticket_Group`, `Ticket`) ,
  INDEX `fk_in_group_ticket_group_idx` (`Ticket_Group` ASC) ,
  INDEX `fk_in_group_ticket_idx` (`Ticket` ASC) ,
  CONSTRAINT `fk_in_group_ticket_group`
    FOREIGN KEY (`Ticket_Group` )
    REFERENCES `mydb`.`ticket_group` (`TGroupId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_in_group_ticket`
    FOREIGN KEY (`Ticket` )
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_log`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_log` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_log` (
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
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_ticket_log_ticket_user1`
    FOREIGN KEY (`Author` )
    REFERENCES `mydb`.`ticket_user` (`TUserId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`support_group`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`support_group` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`support_group` (
  `SGroupId` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Name` VARCHAR(22) NOT NULL ,
  `Tag` VARCHAR(7) NOT NULL ,
  `GroupEmail` VARCHAR(45) NULL ,
  `IMAP_MailServer` VARCHAR(60) NULL ,
  `IMAP_Username` VARCHAR(45) NULL ,
  `IMAP_Password` VARCHAR(45) NULL ,
  PRIMARY KEY (`SGroupId`) ,
  UNIQUE INDEX `Name_UNIQUE` (`Name` ASC) ,
  UNIQUE INDEX `Tag_UNIQUE` (`Tag` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`in_support_group`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`in_support_group` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`in_support_group` (
  `User` INT(10) UNSIGNED NOT NULL ,
  `Group` INT(10) UNSIGNED NOT NULL ,
  INDEX `fk_in_support_group_ticket_user1` (`User` ASC) ,
  INDEX `fk_in_support_group_support_group1` (`Group` ASC) ,
  CONSTRAINT `fk_in_support_group_ticket_user1`
    FOREIGN KEY (`User` )
    REFERENCES `mydb`.`ticket_user` (`TUserId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_in_support_group_support_group1`
    FOREIGN KEY (`Group` )
    REFERENCES `mydb`.`support_group` (`SGroupId` )
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`forwarded`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`forwarded` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`forwarded` (
  `Group` INT(10) UNSIGNED NOT NULL ,
  `Ticket` INT UNSIGNED NOT NULL ,
  INDEX `fk_forwarded_support_group1` (`Group` ASC) ,
  INDEX `fk_forwarded_ticket1` (`Ticket` ASC) ,
  CONSTRAINT `fk_forwarded_support_group1`
    FOREIGN KEY (`Group` )
    REFERENCES `mydb`.`support_group` (`SGroupId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_forwarded_ticket1`
    FOREIGN KEY (`Ticket` )
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`email`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`email` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`email` (
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
    REFERENCES `mydb`.`ticket_user` (`TUserId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_email_ticket1`
    FOREIGN KEY (`TicketId` )
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_email_support_group1`
    FOREIGN KEY (`Sender` )
    REFERENCES `mydb`.`support_group` (`SGroupId` )
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_info`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_info` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_info` (
  `TInfoId` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `Ticket` INT UNSIGNED NOT NULL ,
  `ShardId` INT NULL ,
  `UserPosition` VARCHAR(65) NULL ,
  `ViewPosition` VARCHAR(65) NULL ,
  `ClientVersion` VARCHAR(65) NULL ,
  `PatchVersion` VARCHAR(65) NULL ,
  `ServerTick` VARCHAR(40) NULL ,
  `ConnectState` VARCHAR(40) NULL ,
  `LocalAddress` VARCHAR(60) NULL ,
  `Memory` VARCHAR(60) NULL ,
  `OS` VARCHAR(120) NULL ,
  `Processor` VARCHAR(120) NULL ,
  `CPUID` VARCHAR(50) NULL ,
  `CpuMask` VARCHAR(50) NULL ,
  `HT` VARCHAR(65) NULL ,
  `NeL3D` VARCHAR(120) NULL ,
  `PlayerName` VARCHAR(45) NULL ,
  `UserId` INT NULL ,
  `TimeInGame` VARCHAR(50) NULL ,
  PRIMARY KEY (`TInfoId`) ,
  INDEX `fk_ticket_info_ticket1` (`Ticket` ASC) ,
  CONSTRAINT `fk_ticket_info_ticket1`
    FOREIGN KEY (`Ticket` )
    REFERENCES `mydb`.`ticket` (`TId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
