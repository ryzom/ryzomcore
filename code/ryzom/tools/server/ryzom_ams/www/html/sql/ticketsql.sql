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
  `TCategoryId` INT NOT NULL AUTO_INCREMENT ,
  `Name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`TCategoryId`) ,
  UNIQUE INDEX `Name_UNIQUE` (`Name` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ams_user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ams_user` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ams_user` (
  `UId` INT(10) NOT NULL AUTO_INCREMENT ,
  `Login` VARCHAR(64) NOT NULL ,
  `Password` VARCHAR(13) NULL DEFAULT NULL ,
  `Email` VARCHAR(255) NOT NULL ,
  `Permission` INT(3) NOT NULL DEFAULT 1 ,
  `FirstName` VARCHAR(80) NOT NULL ,
  `LastName` VARCHAR(80) NOT NULL ,
  `Gender` TINYINT(1) NOT NULL DEFAULT 0 ,
  `Country` CHAR(2) NOT NULL DEFAULT 'AA' ,
  PRIMARY KEY (`UId`) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket` (
  `TId` INT NOT NULL AUTO_INCREMENT ,
  `Date` TIMESTAMP NOT NULL ,
  `Title` VARCHAR(120) NOT NULL ,
  `Status` INT NULL DEFAULT 0 ,
  `Queue` INT NULL DEFAULT 0 ,
  `Ticket_Category` INT NOT NULL ,
  `Author` INT NOT NULL ,
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
    REFERENCES `mydb`.`ams_user` (`UId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`assigned`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`assigned` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`assigned` (
  `Ticket` INT NOT NULL ,
  `User` INT NOT NULL ,
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
    REFERENCES `mydb`.`ams_user` (`UId` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`tag`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`tag` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`tag` (
  `TagId` INT NOT NULL AUTO_INCREMENT ,
  `Value` VARCHAR(60) NOT NULL ,
  PRIMARY KEY (`TagId`) ,
  UNIQUE INDEX `Value_UNIQUE` (`Value` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`tagged`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`tagged` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`tagged` (
  `Ticket` INT NOT NULL ,
  `Tag` INT NOT NULL ,
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
  `TContentId` INT NOT NULL AUTO_INCREMENT ,
  `Content` TEXT NULL ,
  PRIMARY KEY (`TContentId`) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`ticket_reply`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`ticket_reply` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`ticket_reply` (
  `TReplyId` INT NOT NULL AUTO_INCREMENT ,
  `Ticket` INT NOT NULL ,
  `Author` INT NOT NULL ,
  `Content` INT NOT NULL ,
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
    REFERENCES `mydb`.`ams_user` (`UId` )
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
  `TGroupId` INT NOT NULL AUTO_INCREMENT ,
  `Title` VARCHAR(80) NOT NULL ,
  PRIMARY KEY (`TGroupId`) ,
  UNIQUE INDEX `Title_UNIQUE` (`Title` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`in_group`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`in_group` ;

CREATE  TABLE IF NOT EXISTS `mydb`.`in_group` (
  `Ticket_Group` INT NOT NULL ,
  `Ticket` INT NOT NULL ,
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



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
