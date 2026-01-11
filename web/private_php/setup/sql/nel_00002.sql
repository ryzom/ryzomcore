ALTER TABLE `permission` CHANGE `ClientApplication` `ClientApplication` CHAR( 64 ) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL ;
ALTER TABLE `permission` DROP `prim` ;
ALTER TABLE `permission` ADD `PermissionId` INT NOT NULL AUTO_INCREMENT PRIMARY KEY FIRST ;
ALTER TABLE `permission` ADD `DomainId` INT NOT NULL DEFAULT '-1' AFTER `UId` ;
ALTER TABLE `permission` DROP `ClientApplication` ;
