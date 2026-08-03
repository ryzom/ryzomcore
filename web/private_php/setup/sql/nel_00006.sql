ALTER TABLE `user`
  CHANGE `State` `State` ENUM('Offline','Authorized','Waiting','Online') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'Offline',
  ADD `Cookie` VARCHAR(30) CHARACTER SET ascii COLLATE ascii_bin NOT NULL DEFAULT '' AFTER `State`;
