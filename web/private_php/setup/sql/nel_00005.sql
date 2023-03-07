ALTER TABLE `domain`
  CHANGE `domain_name` `domain_name` VARCHAR(32) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `status` `status` ENUM('ds_close','ds_dev','ds_restricted','ds_open') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ds_dev',
  CHANGE `ring_db_name` `ring_db_name` VARCHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '';

ALTER TABLE `permission`
  CHANGE `AccessPrivilege` `AccessPrivilege` SET('OPEN','DEV','RESTRICTED') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'OPEN';

ALTER TABLE `shard`
  CHANGE `Name` `Name` VARCHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT 'unknown shard',
  CHANGE `Version` `Version` VARCHAR(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `State` `State` ENUM('ds_close','ds_dev','ds_restricted','ds_open') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ds_dev';

ALTER TABLE `user`
  CHANGE `Password` `Password` VARCHAR(106) CHARACTER SET ascii COLLATE ascii_bin NULL DEFAULT NULL,
  CHANGE `State` `State` ENUM('Offline','Online') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'Offline',
  CHANGE `Privilege` `Privilege` VARCHAR(255) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `ExtendedPrivilege` `ExtendedPrivilege` VARCHAR(128) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '';

ALTER TABLE `permission`
  ADD UNIQUE KEY `UserShard` (`UId`,`ShardId`) USING BTREE;

ALTER TABLE `user`
  ADD PRIMARY KEY (`UId`),
  ADD KEY `Login` (`Login`);
